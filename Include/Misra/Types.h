/// file      : misra/types.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, Anvie Labs, All rights reserved.
///
/// Common type definitions, macro definitions and other misc utilities

#ifndef MISRA_TYPES_H
#define MISRA_TYPES_H

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;

typedef i8 bool;

#ifndef true
#    define true 1
#endif

#ifndef false
#    define false 0
#endif

#ifndef NULL
#    define NULL 0
#endif

#define NEW(tname) calloc (1, sizeof (tname))
#define FREE(x)    (free ((void *)(x)), (x) = NULL)

#endif // MISRA_TYPES_H
