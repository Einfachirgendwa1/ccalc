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
        Buf buf = {32, 0, 0, calloc(buf.capacity, sizeof(char))};
        char c = 0;
        evalcallstack = 0;

        debuglevel = 0;
        while (c != '\n') {
            Result *res = eval(buf.ptr, 0, buf.size, "");
            char tchar1;
            printf("\033[2K\r\033[E\033[2K\r\033[F"); // Frag mich nicht, ich habs auch vergessen
            CYAN;
            tchar1 = buf.ptr[buf.cpos];
            buf.ptr[buf.cpos] = '\0';
            printf(" > %s\033[s", buf.ptr);
            buf.ptr[buf.cpos] = tchar1;
            printf("%s   ", buf.ptr + buf.cpos);
            TERM;
            if (res->type == DOUBLE) {
                GREEN;
                printf(" = %f", res->data.dval);
            } else {
                char tchar2;
                if (res->data.errinfo.idx <= buf.size) {
                    RED;
                    printf("\n %s\033[F", res->data.errinfo.msg);
                    CYAN;
                    tchar2 = buf.ptr[res->data.errinfo.idx];
                    buf.ptr[res->data.errinfo.idx] = '\0';
                    printf(" > %s", buf.ptr);
                    buf.ptr[res->data.errinfo.idx] = tchar2;
                    RED;
                    printf("%c", tchar2);
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
                            for (u32 x = buf.cpos-- - 1; x < buf.size; x++) {
                                // printf("Pulle %c auf %c\n", buf.ptr[x + 1], buf.ptr[x]);
                                buf.ptr[x] = buf.ptr[x + 1];
                            }
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
                case 12: // Ctrl+L
                    {
                        printf("\x1b[2J\x1b[H");
                        break;
                    }
                case '\n':
                    {
                        break;
                    }
                case '(':
                    {
                        APPEND_CHAR(buf, '(');
                        APPEND_CHAR(buf, ')');
                        buf.cpos--;
                        break;
                    }
                default:
                    {
                        APPEND_CHAR(buf, c);
                        break;
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
