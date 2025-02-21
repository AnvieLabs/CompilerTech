/// file      : selfstart/macros.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// Some helper macros for SelfStart.
///

#ifndef MISRA_SELFSTART_MACROS_H
#define MISRA_SELFSTART_MACROS_H

#define INCLUDE_DIR(x) "-I" x " "
#define LINK_DIR(x)    "-L" x " "

#define SOURCES(...)   ZSTR_ARRAY (__VA_ARGS__)
#define LIBRARIES(...) ZSTR_ARRAY (__VA_ARGS__)
#define FLAGS(...)     ZSTR_ARRAY (__VA_ARGS__)

#define NO_LIBRARIES NULL
#define NO_FLAGS     NULL

#endif
