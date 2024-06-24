#include <math.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;

#pragma clang diagnostic warning "-Weverything"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wmissing-noreturn"

extern struct termios orig_termios;

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
    for (u32 x = 0; x < evalcallstack; x++) {                                                             \
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
        u32 didx = left;                  \
        u32 rbound = right;               \
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

#define RETURNERROR(msg, idx)                              \
    DEBUGPRINT(1, "Fehler aufgetreten, returne Error..."); \
    return err_res(msg, idx)

#define RETURNVALUE(x)              \
    DEBUGPRINT(1, "Returne %f", x); \
    return val_res(x)

#define EPSILON 1e-9

#define FEQ(left, right) fabs(left - right) < EPSILON

extern u32 evalcallstack;
extern u8 debuglevel;

typedef struct Result {
    enum {
        DOUBLE,
        ERROR
    } type;

    union {
        double dval;
        struct {
            char *msg;
            uint32_t idx;
        } errinfo;
    } data;
} Result;

extern struct termios origermios;
void disable_termbuffering(void);
void restore_termbuffering(void);

Result *val_res(double val);
Result *err_res(char *msg, uint32_t idx);
Result *eval(char *buf, u32 left, u32 right, char reason[]);
Result *direct_eval(char buf[32], u32 left, u32 right);

typedef struct {
    u32 capacity;
    u32 size;
    char *ptr;
} Buf;

void exit_main(Buf buf);
