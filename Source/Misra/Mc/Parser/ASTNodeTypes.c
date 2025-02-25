/// file      : misra/mc/astnodetypes.c
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, Anvie Labs, All rights reserved.
///
/// Method definitions to interact with Mc AST node types.

#include <Misra/Mc/Parser/ASTNodeTypes.h>
#include <Misra/Std/File.h>
#include <Misra/Std/Log.h>

static inline McType* basic_type_array_init (
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


McType* basic_type_init (
    McType*         t,
    McBasicTypeKind type_kind,
    McTypeMod       type_mod,
    bool            is_unsigned,
    u8              size_in_bytes
) {
    return basic_type_array_init (t, type_kind, type_mod, is_unsigned, size_in_bytes, 1);
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
static inline bool parser_can_read_n (const McParser* mcp, i64 n) {
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
static inline McParser* parser_move_by_n (McParser* mcp, i64 n) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    if (parser_can_read_n (mcp, n)) {
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

    if (parser_can_read_n (mcp, 1) && (mcp->read_pos[0] == c)) {
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

    if (parser_can_read_n (mcp, n) && !strncmp (mcp->read_pos, cs, n)) {
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
static inline char parser_peek (const McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments");
        return -1;
    }

    if (parser_can_read_n (mcp, 1)) {
        return mcp->read_pos[0];
    }

    return -1;
}

static inline void parser_skip_ws (McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return;
    }

    while (strchr (" \t\r\n\b\f", parser_peek (mcp))) {
        mcp->read_pos++;
    }
}

///
/// Try to parse a integer.
///
/// si[in,out]  : Pointer to a 64-bit signed integer variable to store parsed
///               integer value into.
/// mcp[in,out] : McParser object to parse from.
///
/// SUCCESS : true, mcp advanced to position after integer.
/// FAILURE : false, mcp unchanged.
///
static inline bool parse_integer (u64* si, McParser* mcp) {
    if (!si || !mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    // parse integer
    u64  val     = 0;
    bool has_val = false;
    while (strchr ("0123456789", parser_peek (mcp))) {
        val = val * 10 + (parser_peek (mcp) - '0');
        mcp->read_pos++;
        has_val = true;
    }

    if (has_val) {
        *si = val;
        return true;
    } else {
        mcp->read_pos = start_pos;
        return false;
    }
}

static inline bool parse_basic_type (McType* t, McParser* mcp) {
    if (!t || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    bool            is_unsigned = false;
    McBasicTypeKind bt_kind     = MC_BASIC_TYPE_KIND_INVALID;
    switch (parser_peek (mcp)) {
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
        if (parse_integer (&nbits, mcp)) {
            if (basic_type_init (t, bt_kind, MC_TYPE_MOD_NONE, is_unsigned, nbits)) {
                return true;
            }
        }
    }

    mcp->read_pos = start_pos;
    return false;
}


static inline bool parse_expr14 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr13 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr12 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr11 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr10 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr9 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr8 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr7 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr6 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr5 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr4 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr3 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr2 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr1 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr0 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments/");
        return false;
    }
    return false;
}


static inline bool parse_expr (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    return false;
}

bool McParseProgram (McProgram* prog, McParser* mcp) {
    if (!prog || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;

    while (parser_can_read_n (mcp, 1)) {
        parser_skip_ws (mcp);
        McType type = {0};
        if (!parse_basic_type (&type, mcp)) {
            break;
        }
    }

    if (parser_can_read_n (mcp, 1)) {
        mcp->read_pos = start_pos;
        return false;
    }

    return true;
}

McParser* McParserDeinit (McParser* mcp) {
    if (!mcp) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    StrDeinit (&mcp->code);
    memset (mcp, 0, sizeof (McParser));

    return mcp;
}


McParser* McParserInit (McParser* mcp, const char* src_name) {
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
