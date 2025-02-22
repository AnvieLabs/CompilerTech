#include "Vidyut/SelfStart.h"

SELF_START ({
    ADD_LIBRARY (
        "ctStd",
        SOURCES (
            "Source/Misra/Std/Log.c",
            "Source/Misra/Std/File.c",
            "Source/Misra/Std/Container/Vec.c",
            "Source/Misra/Std/Container/Str.c"
        ),
        NO_LIBRARIES,
        FLAGS ("-ggdb", "-fPIC")
    );

    // Modern C Compiler
    ADD_EXECUTABLE ("mcc", SOURCES ("Main.c"), LIBRARIES ("ctStd"), FLAGS ("-ggdb", "-fPIC"));
});
