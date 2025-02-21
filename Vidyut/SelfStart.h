/// file      : selfstart/selfstart.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// The only header you must include in SelfStartMain.c
/// Provides wrapper macros for SelfStart commands.
///

#ifndef MISRA_SELFSTART_SELFSTART_H
#define MISRA_SELFSTART_SELFSTART_H

#ifndef WITH_DEBUG
#    define WITH_DEBUG 0
#endif

#include "Commands.h"

///
/// Wrapper around the AddExecutable command. Always use this instead of the command directly.
/// This will help generate the compile_commands.json automatically.
///
#define ADD_EXECUTABLE(exec_name, src_names, lib_names, cflags)                                    \
    ccj = AddExecutable (exec_name, src_names, lib_names, cflags, ccj)

#define ADD_LIBRARY(lib_name, src_names, lib_names, cflags)                                        \
    ccj = AddLibrary (lib_name, src_names, lib_names, cflags, ccj)

#define SELF_START(body)                                                                           \
    int main (int argc, char** argv, char** envp) {                                                \
        /* always rebuild self before starting */                                                  \
        RebuildSelf (argc, argv, envp);                                                            \
                                                                                                   \
        /* start compile commands json string */                                                   \
        const char* ccj = Appendf (NULL, "[", " ");                                                \
                                                                                                   \
        {body}                                                                                     \
                                                                                                   \
        /* end compile commands json */                                                            \
        ccj = Appendf (ccj, "]");                                                                  \
                                                                                                   \
        /* write to compile_commands.json */                                                       \
        WriteToFile ("compile_commands.json", ccj);                                                \
                                                                                                   \
        free ((void*)ccj);                                                                         \
    }

#endif
