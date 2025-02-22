#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

// ct
#include <Misra/Std/File.h>
#include <Misra/Std/Log.h>

typedef int32_t i8;
typedef int32_t i16;
typedef int32_t i32;
typedef int32_t i64;

typedef uint32_t u8;
typedef uint32_t u16;
typedef uint32_t u32;
typedef uint32_t u64;

typedef float  f32;
typedef double f64;

typedef enum McTypeMod {
    MC_TYPE_MOD_NONE    = 0,
    MC_TYPE_MOD_CONST   = 1 << 0,
    MC_TYPE_MOD_POINTER = 1 << 1,
} McTypeMod;

typedef enum McBasicTypeKind {
    MC_TYPE_KIND_INVALID = 0,
    MC_TYPE_KIND_INTEGER,
    MC_TYPE_KIND_FLOAT,
} McBasicTypeKind;

///
/// Represents a basic type like integers, floats, char, etc...
///
typedef struct McBasicType {
    /// Is this an integer? or a a float? Or some other basic type
    McBasicTypeKind kind;

    /// Modifiers for this basic type.
    McTypeMod mod;

    /// By default this is false.
    bool is_unsigned;

    /// Number of bytes this type requires in memory
    u8 size;

    /// If it's an array of elements then how many items are there in the array?
    /// By default this size is 1. If array has no specified size at compile time
    /// then it's 0, meaning it'll be considered as a pointer.
    u64 arr_size;
} McType;

typedef struct McParser {
    Str   code;
    char* read_pos;
} McParser;

///
/// Deinitialize McParser object after using.
///
/// mcp[out] : Reference to McParser object to be deinitialized.
///
/// SUCCESS : `mcp`
/// FAILURE : `NULL`
///
static McParser* McParserDeinit (McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    StrDeinit (&mcp->code);
    memset (mcp, 0, sizeof (McParser));

    return mcp;
}

///
/// Initialize a new Modern C Parser object to help read and parse file
/// with given name `src_name`.
///
/// mcp[out]     : Reference to McParser object to be initialized.
/// src_name[in] : Name of C source code to load and parse.
///
/// SUCCESS : `mcp`
/// FAILURE : `NULL`
///
static McParser* McParserInit (McParser* mcp, const char* src_name) {
    if (!mcp || !src_name) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    memset (mcp, 0, sizeof (McParser));

    if (!ReadCompleteFile (
            src_name,
            (void**)&mcp->code.data,
            &mcp->code.length,
            &mcp->code.capacity
        )) {
        LOG_ERROR ("failed to read complete file \"%s\".", src_name);
        McParserDeinit (mcp);
        return NULL;
    }

    mcp->read_pos = mcp->code.data;

    return mcp;
}

int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf (stderr, "usage: mcc <src>\n");
        return 1;
    }

    const char* src_name = argv[1];

    McParser parser = {0};
    if (!McParserInit (&parser, src_name)) {
        LOG_ERROR ("failed to init parser.");
        return 1;
    }

    McParserDeinit (&parser);

    return 0;
}
