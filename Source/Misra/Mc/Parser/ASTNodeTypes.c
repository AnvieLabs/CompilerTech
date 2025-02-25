/// file      : misra/mc/astnodetypes.c
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, Anvie Labs, All rights reserved.
///
/// Method definitions to interact with Mc AST node types.

#include <Misra/Mc/Parser/ASTNodeTypes.h>
#include <Misra/Std/Container/Str.h>
#include <Misra/Std/File.h>
#include <Misra/Std/Log.h>

#define IS_DIGIT(c) ('0' <= (c) && (c) <= '9')
#define IS_UPPER(c) ('A' <= (c) && (c) <= 'Z')
#define IS_LOWER(c) ('a' <= (c) && (c) <= 'z')
#define IS_ALPHA(c) (IS_UPPER (c) || IS_LOWER (c))
#define IS_ALNUM(c) (IS_ALPHA (c) || IS_DIGIT (c))

static inline McType* type_deinit (McType* t) {
    if (!t) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    memset (t, 0, sizeof (McType));

    return t;
}


static inline McExpr* expr_create() {
    return calloc (1, sizeof (McExpr));
}


static inline void expr_destroy (McExpr* e) {
    if (!e) {
        LOG_ERROR ("invalid arguments.");
        return;
    }

    switch (e->expr_type) {
        case MC_EXPR_TYPE_ADD :
        case MC_EXPR_TYPE_SUB :
        case MC_EXPR_TYPE_MUL :
        case MC_EXPR_TYPE_DIV :
        case MC_EXPR_TYPE_AND :
        case MC_EXPR_TYPE_OR :
        case MC_EXPR_TYPE_XOR :
        case MC_EXPR_TYPE_MOD :
        case MC_EXPR_TYPE_SHR :
        case MC_EXPR_TYPE_SHL :
        case MC_EXPR_TYPE_LE :
        case MC_EXPR_TYPE_GE :
        case MC_EXPR_TYPE_LT :
        case MC_EXPR_TYPE_GT :
        case MC_EXPR_TYPE_EQ :
        case MC_EXPR_TYPE_NE :
        case MC_EXPR_TYPE_LOG_AND :
        case MC_EXPR_TYPE_LOG_OR :
        case MC_EXPR_TYPE_ASSIGN :
        case MC_EXPR_TYPE_ADD_ASSIGN :
        case MC_EXPR_TYPE_SUB_ASSIGN :
        case MC_EXPR_TYPE_MUL_ASSIGN :
        case MC_EXPR_TYPE_DIV_ASSIGN :
        case MC_EXPR_TYPE_MOD_ASSIGN :
        case MC_EXPR_TYPE_AND_ASSIGN :
        case MC_EXPR_TYPE_OR_ASSIGN :
        case MC_EXPR_TYPE_XOR_ASSIGN :
        case MC_EXPR_TYPE_SHR_ASSIGN :
        case MC_EXPR_TYPE_SHL_ASSIGN :
        case MC_EXPR_TYPE_CALL :
        case MC_EXPR_TYPE_ARR_SUB :
        case MC_EXPR_TYPE_ACCESS :
        case MC_EXPR_TYPE_PTR_ACCESS : {
            expr_destroy (e->add.l);
            expr_destroy (e->add.r);
            memset (e, 0, sizeof (McExpr));
            return;
        }


        case MC_EXPR_TYPE_LOG_NOT :
        case MC_EXPR_TYPE_NOT :
        case MC_EXPR_TYPE_UN_PLUS :
        case MC_EXPR_TYPE_UN_MINUS :
        case MC_EXPR_TYPE_IN_PARENS :
        case MC_EXPR_TYPE_ADDR :
        case MC_EXPR_TYPE_DEREF :
        case MC_EXPR_TYPE_SIZE_OF :
        case MC_EXPR_TYPE_ALIGN_OF :
        case MC_EXPR_TYPE_INC_PFX :
        case MC_EXPR_TYPE_INC_SFX :
        case MC_EXPR_TYPE_DEC_PFX :
        case MC_EXPR_TYPE_DEC_SFX : {
            expr_destroy (e->not.e);
            memset (e, 0, sizeof (McExpr));
            return;
        }

        case MC_EXPR_TYPE_CAST : {
            expr_destroy (e->cast.e);
            type_deinit (&e->cast.type);
            memset (e, 0, sizeof (McExpr));
            return;
        }

        case MC_EXPR_TYPE_TERN : {
            expr_destroy (e->tern.c);
            expr_destroy (e->tern.t);
            expr_destroy (e->tern.f);
            memset (e, 0, sizeof (McExpr));
            return;
        }
        case MC_EXPR_TYPE_LIST : {
            VecForeach (&e->list, xpr, { expr_destroy (xpr); });
            VecDeinit (&e->list);
            memset (e, 0, sizeof (McExpr));
            return;
        }
        case MC_EXPR_TYPE_ID : {
            StrDeinit (&e->id);
            memset (e, 0, sizeof (McExpr));
            return;
        }
        case MC_EXPR_TYPE_NUM : {
            memset (e, 0, sizeof (McExpr));
            return;
        }
        default : {
            LOG_ERROR ("unreachable code reached : invalid expression type.");
            memset (e, 0, sizeof (McExpr));
            return;
        }
    }
}

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

static inline bool parse_int (u64* si, McParser* mcp) {
    if (!si || !mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

    // parse integer
    u64  val = 0;
    char c   = parser_peek (mcp);
    while (IS_DIGIT (c)) {
        val = val * 10 + (parser_peek (mcp) - '0');

        mcp->read_pos++;
        c = parser_peek (mcp);
    }

    if (start_pos != mcp->read_pos) {
        *si = val;
        return true;
    } else {
        return false;
    }
}


static inline bool parse_flt (f64* f, McParser* mcp) {
    if (!f || !mcp) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);


    // parse integer
    f64  val = 0;
    char c   = parser_peek (mcp);
    while (IS_DIGIT (c)) {
        val = val * 10 + (parser_peek (mcp) - '0');

        mcp->read_pos++;
        c = parser_peek (mcp);
    }

    parser_skip_ws (mcp);

    if (parser_peek (mcp) != '.') {
        mcp->read_pos = start_pos;
        return false;
    }

    mcp->read_pos++;
    parser_skip_ws (mcp);

    f64 pow = 10;
    f64 dec = 0;

    while (IS_DIGIT (c)) {
        dec = dec + (parser_peek (mcp) - '0') / pow;
        pow = pow * 10;

        mcp->read_pos++;
        c = parser_peek (mcp);
    }

    val = val + dec;

    parser_skip_ws (mcp);

    if (parser_peek (mcp) == 'f') {
        mcp->read_pos++;
    }

    if (start_pos != mcp->read_pos) {
        *f = val;
        return true;
    } else {
        return false;
    }
}


static inline bool parse_basic_type (McType* t, McParser* mcp) {
    if (!t || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

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
        if (parse_int (&nbits, mcp)) {
            if (basic_type_init (t, bt_kind, MC_TYPE_MOD_NONE, is_unsigned, nbits)) {
                return true;
            }
        }
    }

    mcp->read_pos = start_pos;
    return false;
}


static inline bool parse_id (Str* id, McParser* mcp) {
    if (!id || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

    char c = parser_peek (mcp);

    StrInit (id);
    while (c == '_' || IS_ALPHA (c) || (id->length && IS_DIGIT (c))) {
        StrPushBack (id, c);
        mcp->read_pos++;
        c = parser_peek (mcp);
    }

    if (!id->length) {
        return false;
    }

    return true;
}


static inline bool parse_expr14 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr13 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr12 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr11 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr10 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr9 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr8 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr7 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr6 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr5 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr4 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr3 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr2 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }
    return false;
}


static inline bool parse_expr1 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    return false;
}


static inline bool parse_expr0 (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

    if (parse_expr1 (e, mcp)) {
        parser_skip_ws (mcp);

        struct {
            const char* opnd;
            u8          opnd_len;
            McExprType  expr_type;
        } opnd_type[] = {
            {  "=", 1,     MC_EXPR_TYPE_ASSIGN},
            { "+=", 2, MC_EXPR_TYPE_ADD_ASSIGN},
            { "-=", 2, MC_EXPR_TYPE_SUB_ASSIGN},
            { "*=", 2, MC_EXPR_TYPE_MUL_ASSIGN},
            { "/=", 2, MC_EXPR_TYPE_DIV_ASSIGN},
            { "%=", 2, MC_EXPR_TYPE_MOD_ASSIGN},
            { "&=", 2, MC_EXPR_TYPE_AND_ASSIGN},
            { "|=", 2,  MC_EXPR_TYPE_OR_ASSIGN},
            { "^=", 2, MC_EXPR_TYPE_XOR_ASSIGN},
            {">>=", 3, MC_EXPR_TYPE_SHR_ASSIGN},
            {"<<=", 3, MC_EXPR_TYPE_SHL_ASSIGN},
        };
        u64 nopnds = sizeof (opnd_type) / sizeof (opnd_type[0]);

        McExpr* r = expr_create();

        for (u64 o = 0; o < nopnds; o++) {
            if (parser_can_read_n (mcp, opnd_type[o].opnd_len) &&
                !strncmp (mcp->read_pos, opnd_type[o].opnd, opnd_type[o].opnd_len)) {
                parser_skip_ws (mcp);

                mcp->read_pos++;

                if (parse_expr1 (r, mcp)) {
                    parser_skip_ws (mcp);

                    McExpr* l = expr_create();
                    memcpy (l, e, sizeof (McExpr));

                    e->expr_type = opnd_type[o].expr_type;
                    e->assign.l  = l;
                    e->assign.r  = r;

                    return true;
                }
            }
        }

        // a direct pass to expr1
        expr_destroy (r);
        return true;
    }

    return false;
}


static inline bool parse_expr (McExpr* e, McParser* mcp) {
    if (!e || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

    Str id = {0};
    if (parse_id (&e->id, mcp)) {
        e->expr_type = MC_EXPR_TYPE_ID;
        return true;
    }

    if (!(e->num.is_int = !parse_flt (&e->num.f, mcp))) {
        e->expr_type = MC_EXPR_TYPE_NUM;
        return true;
    } else if ((e->num.is_int = parse_int (&e->num.i, mcp))) {
        e->expr_type = MC_EXPR_TYPE_NUM;
        return true;
    }

    // NOTE(brightprogrammer): Passing expr directly here
    // can this create problems?
    if (parser_peek (mcp) == '(') {
        mcp->read_pos++;

        parser_skip_ws (mcp);
        if (!parse_expr (e, mcp)) {
            mcp->read_pos = start_pos;
            return false;
        }
        parser_skip_ws (mcp);

        if (parser_peek (mcp) == ')') {
            mcp->read_pos++;
        } else {
            mcp->read_pos = start_pos;
            return false;
        }

        return true;
    }

    if (parse_expr0 (e, mcp)) {
        parser_skip_ws (mcp);

        // if we get a comma, then this is actually a list expression
        if (parser_peek (mcp) == ',') {
            // create a clone of currently parse expr
            McExpr* ep = expr_create();
            *ep        = *e;

            // change current expr's type to list
            e->expr_type = MC_EXPR_TYPE_LIST;
            VecInit (&e->list, NULL, NULL);
            VecPushBack (&e->list, &ep);

            while (parser_peek (mcp) == ',') {
                mcp->read_pos++;
                parser_skip_ws (mcp);

                // make space for one more expression
                VecResize (&e->list, e->list.length + 1);
                VecLast (&e->list) = expr_create();

                parse_expr (VecLast (&e->list), mcp);
            }
        } else {
            // otherwise this was just passed to expr0 directly
            return true;
        }
    }

    return false;
}

bool McParseProgram (McProgram* prog, McParser* mcp) {
    if (!prog || !mcp) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = mcp->read_pos;
    parser_skip_ws (mcp);

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
