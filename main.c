#include "lib.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint8_t debuglevel = 1;

int main(void) {
    struct {
        uint32_t capacity;
        uint32_t size;
        char *ptr;
    } buf = {32, 0, malloc(sizeof(char) * buf.capacity)};
    Result *result;
    uint32_t idx = 0;

    DEBUGPRINT(1, "Debugprinting ist auf Level %d", debuglevel);
    DEBUGPRINT(2, "Stoppe Terminal Buffering.");
    disable_termbuffering();
    while (1) {
        char c = 0;
        evalcallstack = 0;
        CYAN;

        while (c != '\n') {
            printf("\033[2K\r");
            printf(" > %s", buf.ptr);
            fflush(stdout);
            if (read(STDIN_FILENO, &c, 1) == 0) {
                restore_termbuffering();
                printf("test");
                free(buf.ptr);
                TERM;
                printf("\n");
                return 0;
            }
            if (c != '\n') {
                if (buf.size + 1 > buf.capacity) {
                    buf.capacity *= 2;
                    buf.ptr = realloc(buf.ptr, sizeof(char) * buf.capacity);
                }
                buf.ptr[buf.size] = c;
                buf.size++;
                buf.ptr[buf.size] = '\0';
            }
        }

        TERM;

        if (strcmp(buf.ptr, "dbg+") == 0) {
            debuglevel++;
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel - 1, debuglevel);
            continue;
        } else if (strcmp(buf.ptr, "dbg-") == 0) {
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel, debuglevel - 1);
            debuglevel--;
            continue;
        }

        result = eval(buf.ptr, idx, buf.size, "");

        if (result->type == DOUBLE) {
            GREEN;
            printf("%s = %f", buf.ptr, result->data.dval);
        } else {
            RED;
            printf("%s", result->data.msg);
        }
        COLOREND;
        free(buf.ptr);
        free(result);
    }
}
