#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "err.h"

int syserr(int bl, const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr, " (%d; %s)\n", bl, strerror(bl));
    return bl;
}


int fatal(const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);

    fprintf(stderr, "\n");
    return -1;
}
