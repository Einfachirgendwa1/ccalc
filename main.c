#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 1e-9

#define FEQ(left, right) fabs(left - right) < EPSILON

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wpadded"

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

    while (1) {
        printf(" > ");
        if (fgets(buf, sizeof(buf), stdin) == 0) {
            printf("\n");
            return 0;
        }
        buf[strlen(buf) - 1] = '\0';
        result = eval(buf, 0, (uint32_t)strlen(buf));

        if (result->type == ERROR) {
            printf("%s\n", result->data.msg);
        } else {
            printf("%s = %f\n", buf, result->data.dval);
        }
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
                    return err_res("Rechenzeichen ohne Zahl davor.");
                }
            }
            if (c == '(') {
                uint32_t start = idx + 1;
                Result *res;
                while (c != ')') {
                    c = buf[++idx];
                    if (c == '\0') {
                        return err_res("Klammer wurde nicht korrekt geschlossen");
                    }
                }
                res = eval(buf, start, idx);
                if (res->type == ERROR) {
                    return res;
                }
                if (!left_value.exists) {
                    return val_res(left_value.dval * res->data.dval);
                } else {
                    left_value.dval = res->data.dval;
                    free(res);
                }
                continue;
            }
            right_res = eval(buf, idx + 1, right);
            if (right_res->type == ERROR) {
                return right_res;
            }
            right_value = right_res->data.dval;
            free(right_res);
            switch (c) {
                case '+':
                    return val_res(left_value.dval + right_value);
                case '-':
                    return val_res(left_value.dval - right_value);
                case '*':
                    return val_res(left_value.dval * right_value);
                case '/':
                    if (FEQ(right_value, 0.0)) {
                        return err_res("Division durch 0");
                    }
                    return val_res(left_value.dval / right_value);
                default:
                    return err_res("Unerwartes Rechenzeichen gefunden.");
            }
        }
    }
    if (!left_value.exists) {
        return err_res("Expression erwartet, aber nichts gefunden.");
    }
    return val_res(left_value.dval);
}
