#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void output(char **argv, const char *name, char type, ...)
#ifdef __GNUC__
    __attribute__((__sentinel__(0)))
#endif
;
extern void write_result(const floatType *x, int n);
#ifdef __cplusplus
}
#endif
