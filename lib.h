#include <math.h>
#include <stdint.h>

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"

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

#define DBGSTART                                                                                          \
    YELLOW;                                                                                               \
    for (uint32_t x = 0; x < evalcallstack; x++) {                                                        \
        putchar(' ');                                                                                     \
    }                                                                                                     \
    switch (evalcallstack) {                                                                              \
        case 0:                                                                                           \
            LPAD(PADDING - (int)evalcallstack, "%d in %s in func main", __LINE__, __FILE__);              \
            break;                                                                                        \
        case 1:                                                                                           \
            LPAD(PADDING - (int)evalcallstack, "%d in %s in func eval", __LINE__, __FILE__);              \
            break;                                                                                        \
        default:                                                                                          \
            LPAD(PADDING - (int)evalcallstack, "%d in %s in eval %d", __LINE__, __FILE__, evalcallstack); \
            break;                                                                                        \
    }                                                                                                     \
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
#define PRINTBUFFERAREA(left, right, buf) \
    do {                                  \
        uint32_t didx = left;             \
        uint32_t rbound = right;          \
        while (isspace(buf[didx])) {      \
            didx++;                       \
        }                                 \
        while (isspace(buf[rbound])) {    \
            rbound--;                     \
        }                                 \
        for (; didx <= rbound; didx++) {  \
            printf("%c", buf[didx]);      \
        }                                 \
                                          \
    } while (0)

#define FMTBOOL(b) b ? "true" : "false"

#define RETURNERROR(x)                                     \
    DEBUGPRINT(1, "Fehler aufgetreten, returne Error..."); \
    return err_res(x)

#define RETURNVALUE(x)              \
    DEBUGPRINT(1, "Returne %f", x); \
    return val_res(x)

#define EPSILON 1e-9

#define FEQ(left, right) fabs(left - right) < EPSILON

extern uint32_t evalcallstack;
extern uint8_t debuglevel;

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

extern struct termios orig_termios;
void disable_termbuffering(void);
void restore_termbuffering(void);

Result *val_res(double val);
Result *err_res(char *msg);
Result *eval(char *buf, uint32_t left, uint32_t right, char reason[]);
Result *direct_eval(char buf[32], uint32_t left, uint32_t right);
