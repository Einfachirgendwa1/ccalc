#include "lib.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

u8 debuglevel = 1;

int main(void) {
    Result *result;
    u32 idx = 0;

    DEBUGPRINT(1, "Debugprinting ist auf Level %d", debuglevel);
    DEBUGPRINT(2, "Stoppe Terminal Buffering.");
    disable_termbuffering();
    while (1) {
        Buf buf = {32, 0, calloc(buf.capacity, sizeof(char))};
        char c = 0;
        evalcallstack = 0;

        debuglevel = 0;
        while (c != '\n') {
            Result *res = eval(buf.ptr, 0, buf.size, "");
            printf("\033[2K\r\033[E\033[2K\r\033[F"); // Frag mich nicht, ich habs auch vergessen
            CYAN;
            printf(" > %s\033[s    ", buf.ptr);
            TERM;
            if (res->type == DOUBLE) {
                GREEN;
                printf(" = %f", res->data.dval);
            } else {
                char tchar;
                if (res->data.errinfo.idx <= buf.size) {
                    RED;
                    printf("\n %s\033[F", res->data.errinfo.msg);
                    CYAN;
                    tchar = buf.ptr[res->data.errinfo.idx];
                    buf.ptr[res->data.errinfo.idx] = '\0';
                    printf(" > %s", buf.ptr);
                    buf.ptr[res->data.errinfo.idx] = tchar;
                    RED;
                    printf("%c", tchar);
                    CYAN;
                    printf("%s ", buf.ptr + res->data.errinfo.idx + 1);
                } else {
                    RED;
                    printf("\n %s\033[F", res->data.errinfo.msg);
                    TERM;
                }
            }

            TERM;
            printf("\033[u");
            fflush(stdout);
            free(res);
            if (read(STDIN_FILENO, &c, 1) == 0) {
                exit_main(buf);
            }
            switch (c) {
                case 127: // Delete Key
                    {
                        if (buf.size > 0) {
                            buf.ptr[--buf.size] = '\0';
                        }
                        break;
                    }
                case 21: // C+U
                    {
                        buf.ptr[0] = '\0';
                        buf.size = 0;
                        break;
                    }
                case 4: // Ctrl+D
                    {
                        exit_main(buf);
                        break;
                    }
                case '\n':
                    {
                        break;
                    }
                default:
                    {
                        if (buf.size + 1 > buf.capacity) {
                            buf.capacity *= 2;
                            buf.ptr = realloc(buf.ptr, sizeof(char) * buf.capacity);
                        }
                        buf.ptr[buf.size] = c;
                        buf.size++;
                        buf.ptr[buf.size] = '\0';
                    }
            }
        }
        debuglevel = 1;

        COLOREND;

        if (strcmp(buf.ptr, "dbg+") == 0) {
            debuglevel++;
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel - 1, debuglevel);
            continue;
        } else if (strcmp(buf.ptr, "dbg-") == 0) {
            DEBUGPRINT(0, "Debuglevel %d -> %d", debuglevel, debuglevel - 1);
            debuglevel--;
            continue;
        } else if (strcmp(buf.ptr, "exit") == 0) {
            printf("\033[2K\r\033[F");
            exit_main(buf);
        }

        result = eval(buf.ptr, idx, buf.size, "");

        if (result->type == DOUBLE) {
            GREEN;
            printf("%s = %f", buf.ptr, result->data.dval);
        } else {
            RED;
            printf("%s", result->data.errinfo.msg);
        }
        COLOREND;
        free(buf.ptr);
        free(result);
    }
}
