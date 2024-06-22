#include "lib.c"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t debuglevel = 2;

int main(void) {
    char buf[32];
    Result *result;
    uint32_t idx = 0;

    DEBUGPRINT(1, "Debugprinting ist auf Level %d", debuglevel);
    while (1) {
        evalcallstack = 0;
        CYAN;
        printf(" > ");
        if (fgets(buf, sizeof(buf), stdin) == 0) {
            TERM;
            printf("\n");
            return 0;
        }
        TERM;
        buf[strlen(buf) - 1] = '\0';

        if (strcmp(buf, "dbg+") == 0) {
            debuglevel++;
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel - 1, debuglevel);
            continue;
        } else if (strcmp(buf, "dbg-") == 0) {
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel, debuglevel - 1);
            debuglevel--;
            continue;
        }

        result = eval(buf, idx, (uint32_t)strlen(buf), "");

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
