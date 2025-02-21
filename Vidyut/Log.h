/// file      : selfstart/log.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// Logging macros for SelfStart.
///

#ifndef MISRA_SELFSTART_LOG_H
#define MISRA_SELFSTART_LOG_H

#if WITH_DEBUG
#    define LOG_ERROR(...)                                                                         \
        do {                                                                                       \
            fprintf (stderr, "[-] %s ", __FUNCTION__);                                             \
            fprintf (stderr, __VA_ARGS__);                                                         \
            fputc ('\n', stderr);                                                                  \
        } while (0)


#    define LOG_WARN(...)                                                                          \
        do {                                                                                       \
            fprintf (stderr, "[!] %s ", __FUNCTION__);                                             \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)

#    define LOG_INFO(...)                                                                          \
        do {                                                                                       \
            fprintf (stderr, "[+] %s ", __FUNCTION__);                                             \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)

#    define LOG_CMD(...)                                                                           \
        do {                                                                                       \
            fprintf (stderr, "[C] %s ", __FUNCTION__);                                             \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)
#else
#    define LOG_ERROR(...)                                                                         \
        do {                                                                                       \
            fprintf (stderr, "[-] ");                                                              \
            fprintf (stderr, __VA_ARGS__);                                                         \
            fputc ('\n', stderr);                                                                  \
        } while (0)


#    define LOG_WARN(...)                                                                          \
        do {                                                                                       \
            fprintf (stderr, "[!] ");                                                              \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)

#    define LOG_INFO(...)                                                                          \
        do {                                                                                       \
            fprintf (stderr, "[+] ");                                                              \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)

#    define LOG_CMD(...)                                                                           \
        do {                                                                                       \
            fprintf (stderr, "[C] ");                                                           \
            fprintf (stdout, __VA_ARGS__);                                                         \
            fputc ('\n', stdout);                                                                  \
        } while (0)
#endif

#endif
