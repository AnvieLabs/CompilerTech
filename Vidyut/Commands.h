/// file      : selfstart/commands.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// SelfMake build system commands. Do not use directly!
/// There's always a macro wrapper, just use that. It's easier.

#ifndef MISRA_SELFSTART_COMMANDS_H
#define MISRA_SELFSTART_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ct/selfstart
#include "Config.h"
#include "File.h"
#include "Log.h"
#include "Str.h"

static inline void ExecCmd (const char* cmd) {
    if (!cmd) {
        LOG_ERROR ("Invalid arguments.");
        return;
    }

    puts (cmd);

    int ret = 0;
    if ((ret = system (cmd))) {
        exit (ret);
    }
}

///
/// Instructs the build ExecCmd to build itself, create a child process,
/// run the newly built binary, and exit from old build binary.
///
/// argc[in] : required
/// argv[in] : required
/// envp[in] : required
///
static inline void RebuildSelf (
    int    argc,
    char** argv,
    char** envp
) { /* do not continue if this is already a forked process */
    if (argc < 1 || !argv || !envp || (argc > 1 && strcmp (argv[argc - 1], "--forked") == 0)) {
        return;
    }

    /* build self first */
    LOG_INFO ("Rebuilding self");
    ExecCmd ("mkdir -pv " BUILD_BINARY_DIR);
    ExecCmd ("mkdir -pv " BUILD_LIBRARY_DIR);
    ExecCmd ("mkdir -pv " BUILD_ARCHIVE_DIR);
    ExecCmd ("gcc -O3 -o Make BuildCommands.c");

    /* replace execution with rebuilt binary */
    char* child_argv[] = {argv[0], "--forked", NULL};
    if (execve ("Make", child_argv, envp) == -1) {
        perror ("execve");
        exit (1);
    }
}

///
/// Takes a command string and appends libraries to link with, and compiler flags
/// to be passed to compiler.
///
/// cmd[in,out]    : String to append to.
/// lib_names[in]  : NULL terminated library names vector.
/// comp_flags[in] : NULL terminated compilation flags.
///
static inline const char* AppendLibrariesAndCompilationFlagsToCommand (
    const char*  cmd,
    const char** lib_names,
    const char** comp_flags
) {
    /* pass all library dependency names (if any) */
    if (lib_names) {
        const char** iter = lib_names;
        while (*iter) {
            cmd = Appendf (cmd, " -l%s", *iter++);
        }
    }

    /* give info about global link and include dirs */
    cmd = Appendf (cmd, " %s", GLOBAL_LINK_DIRS);
    cmd = Appendf (cmd, " %s", GLOBAL_INCLUDE_DIRS);

    /* pass all compilation flags (if any) */
    if (comp_flags) {
        const char** iter = comp_flags;
        while (*iter) {
            cmd = Appendf (cmd, " %s", *iter++);
        }
    }

    return cmd;
}

///
/// Create object file for given source file.
///
/// src_name[in]   : Source file to create object file for.
/// lib_names[in]  : Library names to link with.
/// comp_flags[in] : Compilation flags.
/// ccj[out]       : Append compile commands json this string.
///
/// return ccj on success
/// return NULL otherwise
///
static inline const char* CreateObjectFile (
    const char*  src_name,
    const char** lib_names,
    const char** comp_flags,
    const char*  ccj
) {
    if (!src_name || !ccj) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    const char* wd = GetDirFromFilePath (src_name);

    /* create directory first */
    const char* cmd = Appendf (NULL, "mkdir -pv %s/%s", BUILD_TMP_DIR, wd);
    ExecCmd (cmd);
    *(char*)cmd = 0; /* clear */

    /* compile and create .o file */
    cmd = Appendf (
        cmd,
        "gcc -o %s/%s.o -c %s -Wl,-rpath=%s",
        BUILD_TMP_DIR,
        src_name,
        src_name,
        BUILD_LIBRARY_DIR
    );
    cmd = AppendLibrariesAndCompilationFlagsToCommand (cmd, lib_names, comp_flags);

    /* execute command */
    ExecCmd (cmd);

    /* append compile command data */
    /* reference https://releases.llvm.org/8.0.1/tools/clang/docs/JSONCompilationDatabase.html */
    ccj = Appendf (
        ccj,
        "{\"directory\":\"%s\",\"command\":\"%s\",\"file\":\"%s/%s\"},",
        getenv ("PWD"),
        cmd,
        getenv ("PWD"),
        src_name
    );

    free ((void*)wd);
    free ((void*)cmd);

    return ccj;
}

///
/// Add executable
///
/// exec_name[in]  : Name of executable to be built.
/// src_names[in]  : Names of source files to build this executable.
/// lib_names[in]  : Names of libraries to link the executable with.
/// comp_flags[in] : Compilation flags to be passed to compiler.
/// ccj[out]       : Append compile commands json this string.
///
/// return ccj on success
/// return NULL otherwise
///
static inline const char* AddExecutable (
    const char*  exec_name,
    const char** src_names,
    const char** lib_names,
    const char** comp_flags,
    const char*  ccj
) {
    if (!exec_name || !src_names || !ccj) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    /* start cooking up exec generation command */
    const char* cmd = Appendf (
        NULL,
        "gcc -o %s/%s -Wl,-rpath=%s",
        BUILD_BINARY_DIR,
        exec_name,
        BUILD_LIBRARY_DIR
    );

    /* go through each source file name */
    const char** iter = src_names;
    while (*iter) {
        ccj = CreateObjectFile (*iter, lib_names, comp_flags, ccj); /* create object file */
        cmd = Appendf (cmd, " %s/%s.o", BUILD_TMP_DIR, *iter);      /* add for linking */
        iter++;                                                     /* next source file */
    }
    cmd = AppendLibrariesAndCompilationFlagsToCommand (cmd, lib_names, comp_flags);

    /* create executable */
    ExecCmd (cmd);

    free ((void*)cmd);

    return ccj;
}

///
/// Add library
///
/// lib_name[in]   : Name of executable to be built.
/// src_names[in]  : Names of source files to build this executable.
/// lib_names[in]  : Names of libraries to link the executable with.
/// comp_flags[in] : Compilation flags to be passed to compiler.
///
/// return ccj on success
/// return NULL otherwise
///
static inline const char* AddLibrary (
    const char*  lib_name,
    const char** src_names,
    const char** lib_names,
    const char** comp_flags,
    const char*  ccj
) {
    if (!lib_name || !src_names || !ccj) {
        LOG_ERROR ("Invalid arguments.");
        return NULL;
    }

    /* start cooking up archive generation command */
    const char* ar_cmd = Appendf (NULL, "ar rcs %s/lib%s.a", BUILD_ARCHIVE_DIR, lib_name);
    const char* so_cmd = Appendf (NULL, "gcc -shared -o %s/lib%s.so", BUILD_LIBRARY_DIR, lib_name);

    /* go through each source file name */
    const char** iter = src_names;
    while (*iter) {
        ccj    = CreateObjectFile (*iter, lib_names, comp_flags, ccj); /* create object file */
        ar_cmd = Appendf (ar_cmd, " %s/%s.o", BUILD_TMP_DIR, *iter);   /* add for archival */
        so_cmd = Appendf (so_cmd, " %s/%s.o", BUILD_TMP_DIR, *iter);   /* add for shared object */
        iter++;                                                        /* next source file */
    }

    so_cmd = AppendLibrariesAndCompilationFlagsToCommand (so_cmd, lib_names, comp_flags);

    /* create archive */
    ExecCmd (ar_cmd);

    /* create shared object */
    ExecCmd (so_cmd);

    free ((void*)ar_cmd);
    free ((void*)so_cmd);

    return ccj;
}

#endif
