#include "Vidyut/SelfStart.h"

SELF_START ({
    // Standard library
    ADD_LIBRARY (
        "misra_std",
        SOURCES (
            "Source/Misra/Std/Log.c",
            "Source/Misra/Std/File.c",
            "Source/Misra/Std/Container/Vec.c",
            "Source/Misra/Std/Container/Str.c"
        ),
        NO_LIBRARIES,
        FLAGS ("-ggdb -fPIC -Og")
    );

    // Modern C Library
    ADD_LIBRARY (
        "misra_mc",
        SOURCES ("Source/Misra/Mc/Parser/ASTNodeTypes.c"),
        LIBRARIES ("misra_std"),
        FLAGS ("-ggdb -fpic -Og")
    );

    // Modern C Compiler
    ADD_EXECUTABLE (
        "mcc",
        SOURCES ("Main.c"),
        LIBRARIES ("misra_std", "misra_mc"),
        FLAGS ("-ggdb -fPIC -Og")
    );
    
    ADD_EXECUTABLE (
        "expr_test",
        SOURCES ("Test/Expr.c"),
        LIBRARIES ("misra_std", "misra_mc"),
        FLAGS ("-ggdb -fPIC -Og")
    );
});
