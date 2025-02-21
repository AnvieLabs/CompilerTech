/// file      : selfstart/str.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// String utils for SelfStart
///

#ifndef MISRA_SELFSTART_STR_H
#define MISRA_SELFSTART_STR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Log.h"

static inline const char *Appendf (const char *buf, const char *fmtstr, ...) {
    if (!fmtstr) {
        LOG_ERROR ("Error: Invalid arguments");
        return NULL;
    }

    // Calculate the current length of the buffer
    size_t buf_len = (buf) ? strlen (buf) : 0;

    // Calculate the required length for the new content
    va_list args;
    va_start (args, fmtstr);
    va_list args_copy;
    va_copy (args_copy, args); // Create a copy of args
    int needed_len = vsnprintf (NULL, 0, fmtstr, args);
    va_end (args);

    if (needed_len < 0) {
        LOG_ERROR ("Error: vsnprintf failed\n");
        va_end (args_copy);
        return NULL;
    }

    // Resize the buffer if necessary
    unsigned long long new_len = buf_len + needed_len + 1; // +1 for null terminator
    const char        *new_buf = (const char *)realloc ((void *)buf, new_len);
    if (!new_buf) {
        LOG_ERROR ("Error: Failed to reallocate memory\n");
        va_end (args_copy);
        return NULL;
    }

    // Append the formatted string to the buffer using the copied args
    vsnprintf ((char *)(new_buf + buf_len), needed_len + 1, fmtstr, args_copy);
    va_end (args_copy);

    return new_buf;
}

///
/// Define an array of zero-terminated const char arrays on the spot.
///
#define ZSTR_ARRAY(...) ((const char *[]) {__VA_ARGS__, NULL})

#endif
