#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 1e-9

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"

typedef enum {
    Double,
    Error
} Type;

typedef struct {
    Type type;
    void *val;
} Result;

static Result *val_res(double val);
static Result *err_res(char *msg);
static Result *eval(char *buf, uint32_t left, uint32_t right);
inline static void destroy(Result *val);

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

        if (result->type == Error) {
            printf("%s\n", (char *)result->val);
        } else {
            printf("%s = %f\n", buf, *(double *)result->val);
        }
        destroy(result);
    }
}

static Result *val_res(double val) {
    Result *new = malloc(sizeof(Result));
    new->type = Double;
    new->val = malloc(sizeof(double));
    *(double *)new->val = val;
    return new;
}

static Result *err_res(char *msg) {
    uint64_t len = strlen(msg) + 1;
    Result *new = malloc(sizeof(Result));
    new->type = Error;
    new->val = malloc(len * sizeof(char));
    memcpy(new->val, msg, len);
    return new;
}

static Result *eval(char buf[32], uint32_t left, uint32_t right) {
    int32_t numbuf = 0;
    bool numbuf_init = false;
    uint32_t exp = 1;
    for (uint32_t idx = left; idx < right; idx++) {
        char c = buf[idx];

        if (isspace(c)) {
            continue;
        }

        if (c >= '0' && c <= '9') {
            numbuf_init = true;
            numbuf += (uint32_t)(c - '0') * exp;
            exp += 10;
        } else {
            Result *right_res;
            double right_value;
            if (!numbuf_init) {
                return err_res("Rechenzeichen ohne Zahl davor.");
            }
            right_res = eval(buf, idx + 1, right);
            if (right_res->type == Error) {
                return right_res;
            }
            right_value = *(double *)right_res->val;
            destroy(right_res);
            switch (c) {
            case '+':
                return val_res(numbuf + right_value);
            case '-':
                return val_res(numbuf - right_value);
            case '*':
                return val_res(numbuf * right_value);
            case '/':
                if (fabs(right_value - 0.0) < EPSILON) {
                    return err_res("Division durch 0");
                }
                return val_res(numbuf / right_value);
            default:
                return err_res("Unerwartes Rechenzeichen gefunden.");
            }
        }
    }
    return val_res(numbuf);
}

inline static void destroy(Result *val) {
    free(val->val);
    free(val);
}
