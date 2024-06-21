#include "lib.c"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool res_assert_ok(char *buf, double val);
static bool res_assert_err(char *left);

uint8_t debuglevel = 0;

int main(void) {
    struct {
        char *calc;
        double expected;
        bool trace;
    } coks[] = {{"1 + 1", 2, false},
                {"(1 + 1)", 2, false},
                {"(-1 + 1)", 0, false},
                {"(-4 * 3)", -12, false},
                {"(1 + 1) / 4", 0.5, false},
                {"4(1 + 1)", 8, false},
                {"(1 + 1)4", 8, false}};
    struct {
        char *calc;
        bool trace;
    } cerrs[] = {{"(1 + 1) / ", false},
                 {"(1 + 1) / 0", false}};

    bool traces = false;
    for (uint32_t ok = 0; ok < sizeof(coks) / sizeof(coks[0]); ok++) {
        coks[ok].trace = res_assert_ok(coks[ok].calc, coks[ok].expected);
        traces = traces || coks[ok].trace;
    }
    for (uint32_t err = 0; err < sizeof(cerrs) / sizeof(cerrs[0]); err++) {
        cerrs[err].trace = res_assert_err(cerrs[err].calc);
        traces = traces || cerrs[err].trace;
    }
    if (!traces) {
        return EXIT_SUCCESS;
    }
    printf("\n\nDebuglogs:\n");
    debuglevel = 1;
    for (uint32_t ok = 0; ok < sizeof(coks) / sizeof(coks[0]); ok++) {
        if (coks[ok].trace) {
            printf("==== %s ====\n", coks[ok].calc);
            eval(coks[ok].calc, 0, (uint32_t)strlen(coks[ok].calc) - 1, "Debug Backtrace");
            printf("==== %s ====\n\n", coks[ok].calc);
        }
    }
    for (uint32_t err = 0; err < sizeof(cerrs) / sizeof(cerrs[0]); err++) {
        if (coks[err].trace) {
            eval(coks[err].calc, 0, (uint32_t)strlen(coks[err].calc) - 1, "Debug Backtrace");
        }
    }

    return EXIT_FAILURE;
}

static bool res_assert_ok(char *buf, double val) {
    Result *res = eval(buf, 0, (uint32_t)strlen(buf) - 1, "Test");
    if (res->type == ERROR) {
        RED;
        printf("Fehler beim Runnen Tests: %s -> %s (%f erwartet.)", buf, res->data.msg, val);
        COLOREND;
        return true;
    } else {
        if ((FEQ(res->data.dval, val))) {
            GREEN;
            printf("%s = %f ", buf, val);
            COLOREND;
            return false;
        } else {
            RED;
            printf("%s sollte %f ergeben, gibt aber %f zurück", buf, val, res->data.dval);
            COLOREND;
            return true;
        }
    }
}

static bool res_assert_err(char *buf) {
    Result *res = eval(buf, 0, (uint32_t)strlen(buf) - 1, "Test");
    if (res->type == DOUBLE) {
        RED;
        printf("%s sollte einen Fehler erzeugen, stattdessen wird %f zurückgegeben.", buf, res->data.dval);
    } else {
        GREEN;
        printf("%s -> %s", buf, res->data.msg);
    }
    COLOREND;
    return res->type == DOUBLE;
}
