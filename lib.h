#include <math.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;
typedef int8_t i8;

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

#define APPEND_CHAR(buf, c)                                      \
    if (buf.size + 1 > buf.capacity) {                           \
        buf.capacity *= 2;                                       \
        buf.ptr = realloc(buf.ptr, sizeof(char) * buf.capacity); \
        MEMCHECK_NULL(buf.ptr);                                  \
    }                                                            \
    do {                                                         \
        char tmpchar = c;                                        \
        for (u32 x = buf.cpos; x < buf.size; x++) {              \
            char tmpchar2 = buf.ptr[x];                          \
            buf.ptr[x] = tmpchar;                                \
            tmpchar = tmpchar2;                                  \
        }                                                        \
        buf.ptr[buf.size] = tmpchar;                             \
    } while (0);                                                 \
    buf.size++;                                                  \
    buf.cpos++;                                                  \
    buf.ptr[buf.size] = '\0';

#define GET_PRIO(c, prio)  \
    switch (c) {           \
        case '+':          \
        case '-':          \
            {              \
                prio = 0;  \
                break;     \
            }              \
        case '*':          \
        case '/':          \
            {              \
                prio = 1;  \
                break;     \
            }              \
        default:           \
            {              \
                prio = -1; \
            }              \
    }

#define MEMCHECK_NULL(ptr)             \
    if (ptr == NULL) {                 \
        printf("Kein Memory mehr!\n"); \
        exit(1);                       \
    }

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
            u32 idx;
        } errinfo;
    } data;
} Result;

extern struct termios origermios;
void disable_termbuffering(void);
void restore_termbuffering(void);

Result *val_res(double val);
Result *err_res(char *msg, u32 idx);
Result *eval(char *buf, u32 left, u32 right, char reason[]);
Result *direct_eval(char buf[32], u32 left, u32 right);

typedef struct {
    u32 capacity;
    u32 size;
    u32 cpos;
    char *ptr;
} Buf;

void exit_main(Buf buf);
