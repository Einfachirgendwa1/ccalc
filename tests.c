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
    } coks[] = {{"1 + 1", 2},
                {"(1 + 1)", 2},
                {"(-1 + 1)", 0},
                {"(-4 * 3)", -12},
                {"(1 + 1) / 4", 0.5},
                {"4(1 + 1)", 8},
                {"(1 + 1)4", 8}};

    char *cerrs[] = {
        "(1 + 1) / ",
        "(1 + 1) / 0"};

    char *btraces[sizeof coks / sizeof coks[0] + sizeof cerrs / sizeof cerrs[0]];
    bool traced = false;

    for (uint32_t ok = 0; ok < sizeof(coks) / sizeof(coks[0]); ok++) {
        btraces[ok] = res_assert_ok(coks[ok].calc, coks[ok].expected) ? coks[ok].calc : "";
    }
    for (uint32_t err = 0; err < sizeof(cerrs) / sizeof(cerrs[0]); err++) {
        btraces[sizeof(coks) / sizeof(coks[0]) + err] = res_assert_err(cerrs[err]) ? cerrs[err] : "";
    }
    debuglevel = 2;
    for (uint32_t trace = 0; trace < sizeof(coks) / sizeof(coks[0]) + sizeof(cerrs) / sizeof(cerrs[0]); trace++) {
        if (btraces[trace][0] != '\0') {
            if (!traced) {
                printf("\n\nDebug Logs:\n");
                traced = true;
            }
            printf("==== %s ====\n", btraces[trace]);
            evalcallstack = 0;
            free(eval(btraces[trace], 0, (uint32_t)strlen(btraces[trace]), " (Debug Traceback)"));
            printf("==== %s ====\n\n", btraces[trace]);
        }
    }

    if (!traced) {
        printf("Tests erfolgreich!\n");
    }

    return traced;
}

static bool res_assert_ok(char *buf, double val) {
    Result *res = eval(buf, 0, (uint32_t)strlen(buf) - 1, "Test");
    bool rv;
    if (res->type == ERROR) {
        RED;
        printf("Fehler beim Runnen Tests: \"%s\" -> %s (%f erwartet.)", buf, res->data.msg, val);
        COLOREND;
        rv = true;
    } else {
        if ((FEQ(res->data.dval, val))) {
            GREEN;
            printf("\"%s\" = %f ", buf, val);
            COLOREND;
            rv = false;
        } else {
            RED;
            printf("\"%s\" sollte %f ergeben, gibt aber %f zurück", buf, val, res->data.dval);
            COLOREND;
            rv = true;
        }
    }
    free(res);
    return rv;
}

static bool res_assert_err(char *buf) {
    Result *res = eval(buf, 0, (uint32_t)strlen(buf) - 1, "Test");
    if (res->type == DOUBLE) {
        RED;
        printf("\"%s\" sollte einen Fehler erzeugen, stattdessen wird %f zurückgegeben.", buf, res->data.dval);
    } else {
        GREEN;
        printf("\"%s\" -> %s", buf, res->data.msg);
    }
    COLOREND;
    free(res);
    return res->type == DOUBLE;
}
