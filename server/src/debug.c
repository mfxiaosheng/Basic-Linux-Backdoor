#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"

// Prints out debug statements
void debug_print(const char *fmt, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#endif
}

// Prints out debug errors
void debug_err(const char *x) {
#ifdef DEBUG
    perror(x);
#endif
}
