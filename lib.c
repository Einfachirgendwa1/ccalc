#include "lib.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

uint32_t evalcallstack = 0;
struct termios orig_termios;

Result *val_res(double val) {
    Result *new = malloc(sizeof(Result));
    new->type = DOUBLE;
    new->data.dval = val;
    return new;
}

Result *err_res(char *msg) {
    Result *new = malloc(sizeof(Result));
    new->type = ERROR;
    new->data.msg = msg;
    return new;
}

Result *eval(char *buf, uint32_t left, uint32_t right, char reason[]) {
    Result *res;
    if (debuglevel >= 1) {
        DBGSTART;
        printf("Calle Eval");
        if (reason[0] != '\0') {
            printf("%s", reason);
        }
        printf("...");
        COLOREND;
    }
    evalcallstack++;
    while (isspace(buf[left])) {
        left++;
    }
    res = direct_eval(buf, left, right);
    evalcallstack--;
    return res;
}

Result *direct_eval(char buf[32], uint32_t left, uint32_t right) {
    struct {
        double dval;
        bool exists;
        bool neg_vorzeichen_carry;
    } left_value = {0, false, false};

    assert(left <= right);

    if (debuglevel >= 1) {
        DBGSTART;
        printf("Evaluating \"");
        PRINTBUFFERAREA(left, right, buf);
        printf("\"");
        COLOREND;
    }

    for (uint32_t idx = left; idx <= right; idx++) {
        char c = buf[idx];

        if (isspace(c)) {
            continue;
        }

        if (c >= '0' && c <= '9') {
            bool vorz = left_value.dval < 0 || left_value.neg_vorzeichen_carry;
            left_value.exists = true;
            DEBUGPRINT(2, "========= CHARNUM START =========");
            DEBUGPRINT(2, "CHAR %c", c);
            DEBUGPRINT(2, "PRE  left_value.dval = %f", left_value.dval);
            left_value.dval *= 10;
            DEBUGPRINT(2, "VORZ %s", FMTBOOL(vorz))
            if (vorz) {
                left_value.dval -= (c - '0');
            } else {
                left_value.dval += (c - '0');
            }
            DEBUGPRINT(2, "POST left_value.dval = %f", left_value.dval);
            DEBUGPRINT(2, "========= CHARNUM STOP  =========");
        } else {
            Result *right_res;
            double right_value;
            if (!left_value.exists) {
                if (c == '-') {
                    left_value.neg_vorzeichen_carry = !left_value.neg_vorzeichen_carry;
                    DEBUGPRINT(2, "Minuszeichen gefunden, left_value.neg_vorzeichen_carry = %s", FMTBOOL(left_value.neg_vorzeichen_carry));
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
                printf("\"");
                PRINTBUFFERAREA(idx, right, buf);
                printf("\" returnt %f", right_res->data.dval);
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

void disable_termbuffering(void) {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;

    new_termios.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void restore_termbuffering(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
