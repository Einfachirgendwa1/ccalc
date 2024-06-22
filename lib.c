#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t debuglevel;

#define TERM printf("\033[0m")

#define RED printf("\033[31m")
#define YELLOW printf("\033[33m")
#define GREEN printf("\033[32m")
#define CYAN printf("\033[36m")

#define LPAD(count, ...)                     \
    do {                                     \
        int a = count - printf(__VA_ARGS__); \
        for (int x = 0; x < a; x++) {        \
            putchar(' ');                    \
        }                                    \
    } while (0)

#define PADDING 50

#define DBGSTART                                                                     \
    YELLOW;                                                                          \
    switch (evalcallstack) {                                                         \
        case 0:                                                                      \
            LPAD(PADDING, "%d in %s in func main", __LINE__, __FILE__);              \
            break;                                                                   \
        case 1:                                                                      \
            LPAD(PADDING, "%d in %s in func eval", __LINE__, __FILE__);              \
            break;                                                                   \
        default:                                                                     \
            LPAD(PADDING, "%d in %s in eval %d", __LINE__, __FILE__, evalcallstack); \
            break;                                                                   \
    }                                                                                \
    printf("DEBUG ");

#define COLOREND \
    TERM;        \
    printf("\n")
#define DEBUGPRINT(level, ...) \
    if (debuglevel >= level) { \
        DBGSTART;              \
        printf(__VA_ARGS__);   \
        COLOREND;              \
    }
#define PRINTBUFFERAREA(left, right, buf)               \
    for (uint32_t didx = left; didx <= right; didx++) { \
        printf("%c", buf[didx]);                        \
    }

#define RETURNERROR(x)                                     \
    DEBUGPRINT(1, "Fehler aufgetreten, returne Error..."); \
    return err_res(x)

#define RETURNVALUE(x)              \
    DEBUGPRINT(1, "Returne %f", x); \
    return val_res(x)

#define EPSILON 1e-9

#define FEQ(left, right) fabs(left - right) < EPSILON

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"

static uint32_t evalcallstack = 0;

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
static Result *direct_eval(char *buf, uint32_t left, uint32_t right);
static Result *eval(char *buf, uint32_t left, uint32_t right, char reason[]);

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

static Result *eval(char *buf, uint32_t left, uint32_t right, char reason[]) {
    Result *res;
    evalcallstack++;
    if (debuglevel >= 1) {
        DBGSTART;
        printf("Calle Eval");
        if (reason[0] != '\0') {
            printf("%s", reason);
        }
        printf("...");
        COLOREND;
    }
    while (isspace(buf[left])) {
        left++;
    }
    res = direct_eval(buf, left, right);
    evalcallstack--;
    return res;
}

static Result *direct_eval(char buf[32], uint32_t left, uint32_t right) {
    struct {
        double dval;
        bool exists;
    } left_value = {0, false};

    assert(left <= right);

    if (debuglevel >= 1) {
        DBGSTART;
        printf("Evaluating ");
        PRINTBUFFERAREA(left, right, buf);
        COLOREND;
    }

    for (uint32_t idx = left; idx <= right; idx++) {
        char c = buf[idx];

        if (isspace(c)) {
            continue;
        }

        if (c >= '0' && c <= '9') {
            left_value.exists = true;
            DEBUGPRINT(2, "========= CHARNUM START =========");
            DEBUGPRINT(2, "CHAR %c", c);
            DEBUGPRINT(2, "PRE  left_value.dval = %f", left_value.dval);
            left_value.dval *= 10;
            left_value.dval += (c - '0');
            DEBUGPRINT(2, "POST left_value.dval = %f", left_value.dval);
            DEBUGPRINT(2, "========= CHARNUM STOP  =========");
        } else {
            Result *right_res;
            double right_value;
            if (!left_value.exists) {
                if (c == '-') {
                    left_value.dval *= -1;
                    DEBUGPRINT(2, "Minuszeichen gefunden, left_value.dval = %f", left_value.dval);
                    continue;
                }

                if (c == '*' || c == '/') {
                    DEBUGPRINT(1, "%c", c);
                    RETURNERROR("Rechenzeichen ohne Zahl davor.");
                }
            }
            if (c == '(') {
                uint32_t start = idx + 1;
                Result *res;
                DEBUGPRINT(1, "Klammer gefunden.");
                while (c != ')') {
                    c = buf[++idx];
                    if (c == '\0') {
                        RETURNERROR("Klammer wurde nicht korrekt geschlossen");
                    }
                }
                res = eval(buf, start, idx - 1, " um das Innere der Klammer auszurechnen");
                if (res->type == ERROR) {
                    return res;
                }
                DEBUGPRINT(1, "Klammer Evaluation returnt %f", res->data.dval);
                if (left_value.exists) {
                    double data = res->data.dval;
                    free(res);
                    DEBUGPRINT(1, "Implizierte Klammermultiplikation.");
                    RETURNVALUE(left_value.dval * data);
                } else {
                    left_value.dval = res->data.dval;
                    left_value.exists = true;
                    free(res);
                }
                continue;
            }
            evalcallstack += 1;
            if (idx + 1 >= right) {
                if (idx == right && buf[idx] >= '0' && buf[idx] <= '9') {
                    left_value.exists = true;
                    left_value.dval *= 10;
                    left_value.dval += (c - '0');
                }
                DEBUGPRINT(1, "Ende der Eingabe gefunden, returne gesammelte Zahl.");
                if (!left_value.exists) {
                    RETURNERROR("Unbekanntes Zeichen gefunden.");
                }
                RETURNVALUE(left_value.dval);
            }
            idx++;
            right_res = eval(buf, idx, right, " um die rechte Seite der Rechnung herauszufinden");
            if (right_res->type == ERROR) {
                return right_res;
            }
            if (debuglevel >= 1) {
                DBGSTART;
                PRINTBUFFERAREA(idx, right, buf)
                printf("returnt %f", right_res->data.dval);
                COLOREND;
            }
            right_value = right_res->data.dval;
            free(right_res);
            DEBUGPRINT(1, "%f %c %f", right_value, c, left_value.dval)
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
