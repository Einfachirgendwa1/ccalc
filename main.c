#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG true

#define TERM printf("\033[0m")

#define RED printf("\033[31m")
#define YELLOW printf("\033[33m")
#define GREEN printf("\033[32m")
#define CYAN printf("\033[36m")

#define DBGSTART                                                                  \
    YELLOW;                                                                       \
    switch (evalcallstack) {                                                      \
        case 0:                                                                   \
            printf("%d in %s main\t: DEBUG ", __LINE__, __FILE__);                \
            break;                                                                \
        case 1:                                                                   \
            printf("%d in %s eval\t: DEBUG ", __LINE__, __FILE__);                \
            break;                                                                \
        default:                                                                  \
            printf("%d in %s %d\t\t: DEBUG ", __LINE__, __FILE__, evalcallstack); \
            break;                                                                \
    }
#define COLOREND \
    TERM;        \
    printf("\n")
#define DEBUGPRINT(...)      \
    if (DEBUG) {             \
        DBGSTART;            \
        printf(__VA_ARGS__); \
        COLOREND;            \
    }

#define RETURNERROR(x)                                  \
    DEBUGPRINT("Fehler aufgetreten, returne Error..."); \
    return err_res(x)

#define RETURNVALUE(x)           \
    DEBUGPRINT("Returne %f", x); \
    return val_res(x)

#define EPSILON 1e-9

#define FEQ(left, right) fabs(left - right) < EPSILON

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wpadded"

static int evalcallstack = 0;

typedef struct Result {
    enum {
        DOUBLE,
        ERROR
    } type;

    union {
        double dval;
        char *msg;
    } data;
} Result;

static Result *val_res(double val);
static Result *err_res(char *msg);
static Result *eval(char *buf, uint32_t left, uint32_t right);

int main(void) {
    char buf[32];
    Result *result;

    DEBUGPRINT("Debugprinting ist aktiviert.")
    while (1) {
        CYAN;
        printf(" > ");
        if (fgets(buf, sizeof(buf), stdin) == 0) {
            printf("\n");
            return 0;
        }
        TERM;
        buf[strlen(buf) - 1] = '\0';
        result = eval(buf, 0, (uint32_t)strlen(buf));

        if (result->type == DOUBLE) {
            GREEN;
            printf("%s = %f", buf, result->data.dval);
        } else {
            RED;
            printf("%s", result->data.msg);
        }
        COLOREND;
        free(result);
    }
}

static Result *val_res(double val) {
    Result *new = malloc(sizeof(Result));
    new->type = DOUBLE;
    new->data.dval = val;
    return new;
}

static Result *err_res(char *msg) {
    Result *new = malloc(sizeof(Result));
    new->type = ERROR;
    new->data.msg = msg;
    return new;
}

static Result *eval(char buf[32], uint32_t left, uint32_t right) {
    struct {
        double dval;
        bool exists;
    } left_value = {0, false};

    int32_t exp = 1;

    for (uint32_t idx = left; idx < right; idx++) {
        char c = buf[idx];

        if (isspace(c)) {
            continue;
        }

        if (c >= '0' && c <= '9') {
            left_value.exists = true;
            left_value.dval += (c - '0') * exp;
            exp *= 10;
        } else {
            Result *right_res;
            double right_value;
            if (!left_value.exists) {
                if (c == '-') {
                    exp *= -1;
                    continue;
                }

                if (c == '*' || c == '/') {
                    DEBUGPRINT("%c", c)
                    RETURNERROR("Rechenzeichen ohne Zahl davor.");
                }
            }
            if (c == '(') {
                uint32_t start = idx + 1;
                Result *res;
                DEBUGPRINT("Klammer gefunden.")
                while (c != ')') {
                    c = buf[++idx];
                    if (c == '\0') {
                        RETURNERROR("Klammer wurde nicht korrekt geschlossen");
                    }
                }
                evalcallstack += 1;
                res = eval(buf, start, idx);
                evalcallstack -= 1;
                if (res->type == ERROR) {
                    return res;
                }
                DEBUGPRINT("Klammer Evaluation returnt %f", res->data.dval)
                if (left_value.exists) {
                    double data = res->data.dval;
                    free(res);
                    DEBUGPRINT("Implizieter Klammer return.")
                    RETURNVALUE(left_value.dval * data);
                } else {
                    left_value.dval = res->data.dval;
                    left_value.exists = true;
                    free(res);
                }
                continue;
            }
            evalcallstack += 1;
            right_res = eval(buf, idx + 1, right);
            evalcallstack -= 1;
            if (right_res->type == ERROR) {
                return right_res;
            }
            if (DEBUG) {
                DBGSTART
                for (uint32_t didx = idx + 1; didx <= right; didx++) {
                    printf("%c", buf[didx]);
                }
                printf(" returnt %f", right_res->data.dval);
                COLOREND;
            }
            right_value = right_res->data.dval;
            free(right_res);
            DEBUGPRINT("%f %c %f", right_value, c, left_value.dval)
            switch (c) {
                case '+':
                    RETURNVALUE(left_value.dval + right_value);
                case '-':
                    RETURNVALUE(left_value.dval - right_value);
                case '*':
                    RETURNVALUE(left_value.dval * right_value);
                case '/':
                    if (FEQ(right_value, 0.0)) {
                        RETURNERROR("Division durch 0");
                    }
                    RETURNVALUE(left_value.dval / right_value);
                default:
                    RETURNERROR("Unerwartes Rechenzeichen gefunden.");
            }
        }
    }
    if (!left_value.exists) {
        RETURNERROR("Expression erwartet, aber nichts gefunden.");
    }
    RETURNVALUE(left_value.dval);
}
