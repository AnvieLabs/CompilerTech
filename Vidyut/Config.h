/// file      : selfstart/config.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// Contains configuration options for the SelfStart build system.
/// Configuring selfstart is as easy as defining these config values before
/// including it in your SelfStartMain.c

#ifndef MISRA_SELFSTART_CONFIG_H
#define MISRA_SELFSTART_CONFIG_H

#include "Macros.h"

#ifndef PATHSEP
#    define PATHSEP "/"
#endif

#ifndef BUILD_ROOT
#    define BUILD_ROOT "./Build"
#endif

#ifndef BUILD_TMP_DIR
#    define BUILD_TMP_DIR BUILD_ROOT PATHSEP "tmp"
#endif

#ifndef GLOBAL_INCLUDE_DIRS
#    define GLOBAL_INCLUDE_DIRS INCLUDE_DIR ("./Include") INCLUDE_DIR ("/usr/local/include")
#endif

#ifndef GLOBAL_LINK_DIRS
#    define GLOBAL_LINK_DIRS LINK_DIR ("/usr/local/lib") LINK_DIR (BUILD_LIBRARY_DIR)
#endif

#ifndef BUILD_BINARY_DIR
#    define BUILD_BINARY_DIR BUILD_ROOT PATHSEP "bin"
#endif

#ifndef BUILD_LIBRARY_DIR
#    define BUILD_LIBRARY_DIR BUILD_ROOT PATHSEP "lib"
#endif

#ifndef BUILD_ARCHIVE_DIR
#    define BUILD_ARCHIVE_DIR BUILD_ROOT PATHSEP "lib"
#endif

#endif
