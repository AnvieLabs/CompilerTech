#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

// ct
#include <Misra/Std/File.h>
#include <Misra/Std/Log.h>

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

typedef enum McTypeMod {
    MC_TYPE_MOD_NONE    = 0,
    MC_TYPE_MOD_CONST   = 1 << 0,
    MC_TYPE_MOD_POINTER = 1 << 1,
    MC_TYPE_MOD_MASK    = 3
} McTypeMod;

typedef enum McBasicTypeKind {
    MC_BASIC_TYPE_KIND_INVALID = 0,
    MC_BASIC_TYPE_KIND_INTEGER,
    MC_BASIC_TYPE_KIND_FLOAT,
    MC_BASIC_TYPE_KIND_MAX
} McBasicTypeKind;

///
/// Represents a basic type like integers, floats, char, etc...
///
typedef struct McBasicType {
    /// Is this an integer? or a a float? Or some other basic type
    McBasicTypeKind type_kind;

    /// Modifiers for this basic type.
    McTypeMod type_mod;

    /// By default this is false.
    bool is_unsigned;

    /// Number of bits this type requires in memory
    u64 nbits;

    /// If it's an array of elements then how many items are there in the array?
    /// By default this size is 1. If array has no specified size at compile time
    /// then it's 0, meaning it'll be considered as a pointer.
    u64 arr_size;
} McBasicType;

typedef enum McTypeKind {
    MC_TYPE_KIND_INVALID = 0,
    MC_TYPE_KIND_BASIC,
    MC_TYPE_KIND_MAX,
} McTypeKind;

typedef struct McType {
    McTypeKind type_kind;
    union {
        McBasicType basic_type;
    };
} McType;

void McTypePrettyPrint (const McType* type) {
    if (type == NULL) {
        printf ("NULL McType\n");
        return;
    }

    switch (type->type_kind) {
        case MC_TYPE_KIND_INVALID :
            printf ("Invalid Type\n");
            break;

        case MC_TYPE_KIND_BASIC : {
            const McBasicType* basic = &type->basic_type;

            // Print the basic type information
            printf ("Basic Type:\n");

            // Print type kind (Integer, Float, etc.)
            switch (basic->type_kind) {
                case MC_BASIC_TYPE_KIND_INVALID :
                    printf ("  Type Kind: Invalid\n");
                    break;
                case MC_BASIC_TYPE_KIND_INTEGER :
                    printf ("  Type Kind: Integer\n");
                    break;
                case MC_BASIC_TYPE_KIND_FLOAT :
                    printf ("  Type Kind: Float\n");
                    break;
                default :
                    printf ("  Type Kind: Unknown\n");
            }

            // Print unsigned flag
            printf ("  Unsigned: %s\n", basic->is_unsigned ? "True" : "False");

            // Print number of bits
            printf ("  Number of Bits: %llu\n", basic->nbits);

            // Print array size
            if (basic->arr_size == 0) {
                printf ("  Array Size: Pointer (dynamic array)\n");
            } else {
                printf ("  Array Size: %llu\n", basic->arr_size);
            }

            // Print modifiers (e.g., CONST, POINTER)
            printf ("  Modifiers: ");
            if (basic->type_mod & MC_TYPE_MOD_CONST) {
                printf ("CONST ");
            }
            if (basic->type_mod & MC_TYPE_MOD_POINTER) {
                printf ("POINTER ");
            }
            if (basic->type_mod == MC_TYPE_MOD_NONE) {
                printf ("NONE ");
            }
            printf ("\n");

            break;
        }

        default :
            printf ("Unknown Type Kind\n");
            break;
    }
}

McType* McTypeInitBasicTypeArray (
    McType*         t,
    McBasicTypeKind type_kind,
    McTypeMod       type_mod,
    bool            is_unsigned,
    u8              nbits,
    u64             arr_size
) {
    if (!t) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    // perform validation checks
    {
        bool early_return = false;

        if (!type_kind || type_kind >= MC_BASIC_TYPE_KIND_MAX) {
            LOG_ERROR ("invalid value for basic type kind.");
            early_return = true;
        }

        if (type_mod & ~MC_TYPE_MOD_MASK) {
            LOG_ERROR ("invalid value for type modifier.");
            early_return = true;
        }

        if (!nbits) {
            LOG_ERROR ("invalid type size provided.");
            early_return = true;
        }

        if (!arr_size) {
            LOG_ERROR ("invalid array size provided.");
            early_return = true;
        }

        if (early_return) {
            return NULL;
        }
    }

    // invalidate all fields
    memset (t, 0, sizeof (McType));

    // everything's validated! just plug in the values
    t->type_kind              = MC_TYPE_KIND_BASIC;
    t->basic_type.type_kind   = type_kind;
    t->basic_type.type_mod    = type_mod;
    t->basic_type.is_unsigned = is_unsigned;
    t->basic_type.nbits       = nbits;
    t->basic_type.arr_size    = arr_size;

    return t;
}

McType* McTypeInitBasicType (
    McType*         t,
    McBasicTypeKind type_kind,
    McTypeMod       type_mod,
    bool            is_unsigned,
    u8              size_in_bytes
) {
    return McTypeInitBasicTypeArray (t, type_kind, type_mod, is_unsigned, size_in_bytes, 1);
}

typedef struct McParser {
    Str         code;
    const char* read_pos;
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

///
/// Check whether there's enough space to read N bytes.
///
/// The read is bi-directional, meaning negative value for `n`
/// means a read attempt backwards, and a positive value for `n`
/// means a read attempt forwards.
///
/// Backward reading means reading from current read position towards
/// the beginning of string. On the other hand, forward reading is
/// reading from current position towards the string end.
///
///      backwards reading (-ve)
///     ;-----------<<----------;
///     v                       ^
/// ;-------;----------------;-----; ... ------;-----;
/// | begin | .............. | cur | ......... | end |
/// ;-------;----------------;-----; ... ------;-----;
///                             v                  ^
///                             ;-------->>--------;
///                              forward read (+ve)
///
/// mcp[in] : McParser object to check space into.
/// n[in]   : Number of bytes to check for.
///
/// SUCCESS : true
/// FAILURE : false
///
bool McParserCanReadN (const McParser* mcp, i64 n) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    // empty string matches to true always
    if (!n) {
        return true;
    }

    // bounds check
    if ((mcp->read_pos + n) >= mcp->code.data &&
        (mcp->read_pos + n) <= (mcp->code.data + mcp->code.length)) {
        return true;
    }

    return false;
}

///
/// If it's possible to read `n` characters in forward or backward
/// direction, then move read position by that many characters in
/// either direction.
///
/// mcp[in,out] : McParser to move read position into.
/// n[in]       : Number of bytes (+ve or -ve) to move cursor by.
///
/// SUCCESS : mcp, read position in mcp moved by n in either direction.
/// FAILURE : NULL, mcp unchanged
///
McParser* McParserMoveBy (McParser* mcp, i64 n) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    if (McParserCanReadN (mcp, n)) {
        mcp->read_pos += n;
    }

    return false;
}

///
/// Try to read a single character at current read position. If match
/// is success then advance by one position and return true, otherwise
/// don't make any changes to read position and return false.
///
/// mcp[in,out] : McParser object to try reading from.
/// c[in]       : A character to match to.
///
/// SUCCESS : true, read position in mcp is advanced by 1
/// FAILURE : false, mcp is unchanged
///
bool McParserReadChar (McParser* mcp, i8 c) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    if (McParserCanReadN (mcp, 1) && (mcp->read_pos[0] == c)) {
        mcp->read_pos++;
        return true;
    }

    return false;
}

///
/// Try to read a character array of given length at current read position
/// in McParser object. If such array exists, then advance by given length
/// of character array to read past that next time, and return true, otherwise
/// return false indicating that the provided array does not exist at
/// current read position.
///
/// mcp[in,out] : McParser object to try reading from.
/// zs[in]      : A character array to match to.
/// n[in]       : Size of character array in bytes.
///
/// SUCCESS : true, read pos in mcp is advanced by n
/// FAILURE : false, mcp is unchanged
///
bool McParserReadCStr (McParser* mcp, const char* cs, u64 n) {
    if (!mcp || !cs) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    // matching empty string is always true
    if (!n) {
        return true;
    }

    if (McParserCanReadN (mcp, n) && !strncmp (mcp->read_pos, cs, n)) {
        mcp->read_pos += n;
        return true;
    }

    return false;
}

///
/// Try to read a zero-terminated string at current read position
/// in McParser object. If such string exists, then advance by length
/// of string to read past that next time, and return true, otherwise
/// return false indicating that the provided string does not exist at
/// current read position.
///
/// mcp[in,out] : McParser object to try reading from.
/// zs[in]      : Zero-terminated string to match to.
///
/// SUCCESS : true, read pos in mcp is advanced by strlen(zs)
/// FAILURE : false, mcp is unchanged
///
bool McParserReadZStr (McParser* mcp, const char* zs) {
    if (!mcp || !zs) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    return McParserReadCStr (mcp, zs, strlen (zs));
}

///
/// Read a single character without advancing the read position.
/// Using this in conjunction with `McParserMoveBy`, a matching function
/// can match a character optionally and advance to any position depending
/// on that.
///
/// mcp[in] : McParser object to peek character from.
///
/// SUCCESS : Character (>=0) at current read position.
/// FAILURE : -1.
///
char McParserPeek (const McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments");
        return -1;
    }

    if (McParserCanReadN (mcp, 1)) {
        return mcp->read_pos[0];
    }

    return -1;
}

void McParserSkipWS (McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return;
    }

    while (strchr (" \t\r\n\b\f", McParserPeek (mcp))) {
        mcp->read_pos++;
    }
}

///
/// Try to parse a signed integer.
///
/// si[in,out]  : Pointer to a 64-bit signed integer variable to store parsed
///               integer value into.
/// mcp[in,out] : McParser object to parse from.
///
/// SUCCESS : true, mcp advanced to position after integer.
/// FAILURE : false, mcp unchanged.
///
bool McParseSignedInteger (i64* si, McParser* mcp) {
    if (!si || !mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    // read sign (optional) and skip any whitespace
    i64 sign = McParserReadChar (mcp, '-') ? -1 : 1;
    McParserSkipWS (mcp);

    // parse integer
    i64  val     = 0;
    bool has_val = false;
    while (strchr ("0123456789", McParserPeek (mcp))) {
        val = val * 10 + (McParserPeek (mcp) - '0');
        mcp->read_pos++;
        has_val = true;
    }

    if (has_val) {
        *si = sign * val;
        return true;
    } else {
        mcp->read_pos = start_pos;
        return false;
    }
}

///
/// Try to parse an unsigned integer.
///
/// ui[in,out]  : Pointer to a 64-bit unsigned integer variable to store parsed
///               integer value into.
/// mcp[in,out] : McParser object to parse from.
///
/// SUCCESS : true, mcp advanced to position after integer.
/// FAILURE : false, mcp unchanged.
///
bool McParseUnsignedInteger (u64* ui, McParser* mcp) {
    if (!ui || !mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    return McParseSignedInteger ((i64*)ui, mcp);
}

bool McParseBasicType (McType* t, McParser* mcp) {
    if (!t || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    bool            is_unsigned = false;
    McBasicTypeKind bt_kind     = MC_BASIC_TYPE_KIND_INVALID;
    switch (McParserPeek (mcp)) {
        case 'u' : {
            mcp->read_pos++;
            bt_kind     = MC_BASIC_TYPE_KIND_INTEGER;
            is_unsigned = true;
            break;
        }
        case 'i' : {
            mcp->read_pos++;
            bt_kind     = MC_BASIC_TYPE_KIND_INTEGER;
            is_unsigned = false;
            break;
        }
        case 'f' : {
            mcp->read_pos++;
            bt_kind = MC_BASIC_TYPE_KIND_FLOAT;
            break;
        }
        default : {
            return false;
        }
    }

    if (bt_kind) {
        u64 nbits = 0;
        if (McParseUnsignedInteger (&nbits, mcp)) {
            if (McTypeInitBasicType (t, bt_kind, MC_TYPE_MOD_NONE, is_unsigned, nbits)) {
                return true;
            }
        }
    }

    mcp->read_pos = start_pos;
    return false;
}

bool McParseProgram (McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    while (McParserCanReadN (mcp, 1)) {
        McParserSkipWS (mcp);
        McType type = {0};
        if (!McParseBasicType (&type, mcp)) {
            break;
        }
        McTypePrettyPrint (&type);
        putchar ('\n');
    }

    if (McParserCanReadN (mcp, 1)) {
        mcp->read_pos = start_pos;
        return false;
    }

    return true;
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

    printf ("program = %b\n", McParseProgram (&parser));

    McParserDeinit (&parser);

    return 0;
}
