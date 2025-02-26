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


static inline bool parse_type (McType* t, McParser* p) {
    if (!t || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    return false;
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
        case MC_EXPR_TYPE_ARR_SUBSCRIPT :
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
        case MC_EXPR_TYPE_INVALID : {
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
/// p[in] : McParser object to check space into.
/// n[in]   : Number of bytes to check for.
///
/// SUCCESS : true
/// FAILURE : false
///
static inline bool parser_can_read_n (const McParser* p, i64 n) {
    if (!p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    // empty string matches to true always
    if (!n) {
        return true;
    }

    // bounds check
    if ((p->read_pos + n) >= p->code.data && (p->read_pos + n) <= (p->code.data + p->code.length)) {
        return true;
    }

    return false;
}

///
/// If it's possible to read `n` characters in forward or backward
/// direction, then move read position by that many characters in
/// either direction.
///
/// p[in,out] : McParser to move read position into.
/// n[in]       : Number of bytes (+ve or -ve) to move cursor by.
///
/// SUCCESS : p, read position in p moved by n in either direction.
/// FAILURE : NULL, p unchanged
///
static inline McParser* parser_move_by_n (McParser* p, i64 n) {
    if (!p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    if (parser_can_read_n (p, n)) {
        p->read_pos += n;
    }

    return false;
}

///
/// Try to read a single character at current read position. If match
/// is success then advance by one position and return true, otherwise
/// don't make any changes to read position and return false.
///
/// p[in,out] : McParser object to try reading from.
/// c[in]       : A character to match to.
///
/// SUCCESS : true, read position in p is advanced by 1
/// FAILURE : false, p is unchanged
///
bool McParserReadChar (McParser* p, i8 c) {
    if (!p) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    if (parser_can_read_n (p, 1) && (p->read_pos[0] == c)) {
        p->read_pos++;
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
/// p[in,out] : McParser object to try reading from.
/// zs[in]      : A character array to match to.
/// n[in]       : Size of character array in bytes.
///
/// SUCCESS : true, read pos in p is advanced by n
/// FAILURE : false, p is unchanged
///
bool McParserReadCStr (McParser* p, const char* cs, u64 n) {
    if (!p || !cs) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    // matching empty string is always true
    if (!n) {
        return true;
    }

    if (parser_can_read_n (p, n) && !strncmp (p->read_pos, cs, n)) {
        p->read_pos += n;
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
/// p[in,out] : McParser object to try reading from.
/// zs[in]      : Zero-terminated string to match to.
///
/// SUCCESS : true, read pos in p is advanced by strlen(zs)
/// FAILURE : false, p is unchanged
///
bool McParserReadZStr (McParser* p, const char* zs) {
    if (!p || !zs) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    return McParserReadCStr (p, zs, strlen (zs));
}

///
/// Read a single character without advancing the read position.
/// Using this in conjunction with `McParserMoveBy`, a matching function
/// can match a character optionally and advance to any position depending
/// on that.
///
/// p[in] : McParser object to peek character from.
///
/// SUCCESS : Character (>=0) at current read position.
/// FAILURE : -1.
///
static inline char parser_peek (const McParser* p) {
    if (!p) {
        LOG_ERROR ("invalid arguments");
        return -1;
    }

    if (parser_can_read_n (p, 1)) {
        return p->read_pos[0];
    }

    return -1;
}

static inline void parser_skip_ws (McParser* p) {
    if (!p) {
        LOG_ERROR ("invalid arguments.");
        return;
    }

    while (strchr (" \t\r\n\b\f", parser_peek (p))) {
        p->read_pos++;
    }
}

static inline bool parse_int (u64* si, McParser* p) {
    if (!si || !p) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    // parse integer
    u64  val = 0;
    char c   = parser_peek (p);
    while (IS_DIGIT (c)) {
        val = val * 10 + (parser_peek (p) - '0');

        p->read_pos++;
        c = parser_peek (p);
    }

    if (start_pos != p->read_pos) {
        *si = val;
        return true;
    } else {
        return false;
    }
}


static inline bool parse_flt (f64* f, McParser* p) {
    if (!f || !p) {
        LOG_ERROR ("invalid arguments");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);


    // parse integer
    f64  val = 0;
    char c   = parser_peek (p);
    while (IS_DIGIT (c)) {
        val = val * 10 + (parser_peek (p) - '0');

        p->read_pos++;
        c = parser_peek (p);
    }

    parser_skip_ws (p);

    if (parser_peek (p) != '.') {
        p->read_pos = start_pos;
        return false;
    }

    p->read_pos++;
    parser_skip_ws (p);

    f64 pow = 10;
    f64 dec = 0;

    while (IS_DIGIT (c)) {
        dec = dec + (parser_peek (p) - '0') / pow;
        pow = pow * 10;

        p->read_pos++;
        c = parser_peek (p);
    }

    val = val + dec;

    parser_skip_ws (p);

    if (parser_peek (p) == 'f') {
        p->read_pos++;
    }

    if (start_pos != p->read_pos) {
        *f = val;
        return true;
    } else {
        return false;
    }
}


static inline bool parse_basic_type (McType* t, McParser* p) {
    if (!t || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    bool            is_unsigned = false;
    McBasicTypeKind bt_kind     = MC_BASIC_TYPE_KIND_INVALID;
    switch (parser_peek (p)) {
        case 'u' : {
            p->read_pos++;
            bt_kind     = MC_BASIC_TYPE_KIND_INTEGER;
            is_unsigned = true;
            break;
        }
        case 'i' : {
            p->read_pos++;
            bt_kind     = MC_BASIC_TYPE_KIND_INTEGER;
            is_unsigned = false;
            break;
        }
        case 'f' : {
            p->read_pos++;
            bt_kind = MC_BASIC_TYPE_KIND_FLOAT;
            break;
        }
        default : {
            return false;
        }
    }

    if (bt_kind) {
        u64 nbits = 0;
        if (parse_int (&nbits, p)) {
            if (basic_type_init (t, bt_kind, MC_TYPE_MOD_NONE, is_unsigned, nbits)) {
                return true;
            }
        }
    }

    p->read_pos = start_pos;
    return false;
}


static inline bool parse_id (Str* id, McParser* p) {
    if (!id || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    char c = parser_peek (p);

    StrInit (id);
    while (c == '_' || IS_ALPHA (c) || (id->length && IS_DIGIT (c))) {
        StrPushBack (id, c);
        p->read_pos++;
        c = parser_peek (p);
    }

    if (!id->length) {
        return false;
    }

    return true;
}

static inline bool parse_expr_list (McExpr* e, McParser* p);
static inline bool parse_expr (McExpr* e, McParser* p);

static inline bool parse_expr_term (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    if (parse_id (&e->id, p)) {
        e->expr_type = MC_EXPR_TYPE_ID;
        return true;
    } else if (!(e->num.is_int = !parse_flt (&e->num.f, p))) {
        e->expr_type = MC_EXPR_TYPE_NUM;
        return true;
    } else if ((e->num.is_int = parse_int (&e->num.i, p))) {
        e->expr_type = MC_EXPR_TYPE_NUM;
        return true;
    }

    return false;
}


static inline bool parse_expr14 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parser_peek (p) == '(') {
        p->read_pos++;
        parser_skip_ws (p);

        if (parse_expr14 (e, p)) {
            parser_skip_ws (p);

            if (parser_peek (p) == ')') {
                p->read_pos++;
                return true;
            }

            p->read_pos = start_pos;
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // fall through
    }

    if (parse_expr_term (e, p)) {
        return true;
    }

    p->read_pos = start_pos;
    return false;
}


static inline bool parse_expr13 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    // (type) { expr_list }
    if (parser_peek (p) == '(') {
        p->read_pos++;
        parser_skip_ws (p);

        if (parse_type (&e->cast.type, p)) {
            if (parser_peek (p) == ')') {
                p->read_pos++;
                parser_skip_ws (p);

                if (parser_peek (p) == '{') {
                    p->read_pos++;
                    parser_skip_ws (p);

                    McExpr* xpr = expr_create();
                    if (parse_expr_list (xpr, p)) {
                        if (parser_peek (p) == '}') {
                            p->read_pos++;

                            e->expr_type = MC_EXPR_TYPE_CAST;
                            e->cast.e    = xpr;
                            return true;
                        }

                        p->read_pos = start_pos;
                        expr_destroy (xpr);
                        memset (e, 0, sizeof (McExpr));
                        return false;
                    }

                    // fall through
                }
                // fall through
            }
            // fall through
        }

        p->read_pos = start_pos;
        memset (e, 0, sizeof (McExpr));
        return false;
    }

    // suffixed expressions
    if (parse_expr14 (e, p)) {
        McExprType et = MC_EXPR_TYPE_INVALID;

        // expr ++
        // expr --
        if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "++", 2)) {
            p->read_pos += 2;
            et           = MC_EXPR_TYPE_INC_SFX;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "--", 2)) {
            p->read_pos += 2;
            et           = MC_EXPR_TYPE_DEC_SFX;
        }

        if (et) {
            parser_skip_ws (p);

            McExpr* xpr  = expr_create();
            *xpr         = *e;
            e->expr_type = et;
            e->inc_pfx.e = xpr;

            return true;
        }

        // expr . expr (member access)
        // expr -> expr (member access through pointer)
        if (parser_peek (p) == '.') {
            p->read_pos++;
            et = MC_EXPR_TYPE_ACCESS;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "->", 2)) {
            p->read_pos += 2;
            et           = MC_EXPR_TYPE_PTR_ACCESS;
        }

        if (et) {
            parser_skip_ws (p);

            McExpr* r = expr_create();

            if (parse_expr14 (r, p)) {
                McExpr* l = expr_create();
                *l        = *e;

                e->expr_type = et;
                e->access.l  = l;
                e->access.r  = r;

                return true;
            }

            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // expr14  ( expr14 ) (function call)
        // expr14  [ expr14 ] (array subscript)
        if (parser_peek (p) == '(') {
            p->read_pos++;
            parser_skip_ws (p);

            McExpr* r = expr_create();
            if (parse_expr14 (r, p)) {
                if (parser_peek (p) == ')') {
                    p->read_pos++;

                    McExpr* l = expr_create();
                    *l        = *e;

                    e->expr_type = MC_EXPR_TYPE_CALL;
                    e->call.l    = l;
                    e->call.r    = r;

                    return true;
                }

                // fall through
            }

            p->read_pos = start_pos;
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        } else if (parser_peek (p) == '[') {
            p->read_pos++;
            parser_skip_ws (p);

            McExpr* r = expr_create();
            if (parse_expr14 (r, p)) {
                if (parser_peek (p) == ']') {
                    p->read_pos++;

                    McExpr* l = expr_create();
                    *l        = *e;

                    e->expr_type       = MC_EXPR_TYPE_ARR_SUBSCRIPT;
                    e->arr_subscript.l = l;
                    e->arr_subscript.r = r;

                    return true;
                }

                // fall through
            }

            p->read_pos = start_pos;
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr12 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    McExprType et = MC_EXPR_TYPE_INVALID;
    if (parser_peek (p) == '+') {
        p->read_pos++;
        et = MC_EXPR_TYPE_UN_PLUS;
    } else if (parser_peek (p) == '-') {
        p->read_pos++;
        et = MC_EXPR_TYPE_UN_MINUS;
    } else if (parser_peek (p) == '!') {
        p->read_pos++;
        et = MC_EXPR_TYPE_LOG_NOT;
    } else if (parser_peek (p) == '~') {
        p->read_pos++;
        et = MC_EXPR_TYPE_NOT;
    } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "++", 2)) {
        p->read_pos += 2;
        et           = MC_EXPR_TYPE_INC_PFX;
    } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "--", 2)) {
        p->read_pos += 2;
        et           = MC_EXPR_TYPE_DEC_PFX;
    } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "sizeof", 6)) {
        p->read_pos += 6;
        et           = MC_EXPR_TYPE_SIZE_OF;
    } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "_Alignof", 8)) {
        p->read_pos += 8;
        et           = MC_EXPR_TYPE_ALIGN_OF;
    }

    if (et) {
        parser_skip_ws (p);

        McExpr* xpr = expr_create();

        e->expr_type = et;
        e->inc_pfx.e = xpr;

        if (parse_expr12 (xpr, p)) {
            return true;
        }

        p->read_pos = start_pos;
        expr_destroy (xpr);
        memset (e, 0, sizeof (McExpr));
        return false;
    }

    // (type) expr type cast
    if (parser_peek (p) == '(') {
        p->read_pos++;
        parser_skip_ws (p);

        if (parse_type (&e->cast.type, p)) {
            if (parser_peek (p) == ')') {
                p->read_pos++;
                parser_skip_ws (p);

                McExpr* xpr = expr_create();
                if (parse_expr12 (xpr, p)) {
                    e->expr_type = MC_EXPR_TYPE_CAST;
                    e->cast.e    = xpr;
                    return true;
                }

                p->read_pos = start_pos;
                expr_destroy (xpr);
                memset (e, 0, sizeof (McExpr));
                return false;
            }

            // fall through
        }

        p->read_pos = start_pos;
        memset (e, 0, sizeof (McExpr));
        return false;
    }

    if (parse_expr13 (e, p)) {
        return true;
    }

    return false;
}


static inline bool parse_expr11 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr12 (e, p)) {
        parser_skip_ws (p);

        McExprType et = MC_EXPR_TYPE_INVALID;
        if (parser_peek (p) == '*') {
            et = MC_EXPR_TYPE_MUL;
        } else if (parser_peek (p) == '/') {
            et = MC_EXPR_TYPE_DIV;
        } else if (parser_peek (p) == '%') {
            et = MC_EXPR_TYPE_MOD;
        }

        if (et) {
            p->read_pos++;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = et;
            e->mul.l     = l;
            e->mul.r     = r;

            if (parse_expr11 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr10 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr11 (e, p)) {
        parser_skip_ws (p);

        McExprType et = MC_EXPR_TYPE_INVALID;
        if (parser_peek (p) == '+') {
            et = MC_EXPR_TYPE_ADD;
        } else if (parser_peek (p) == '-') {
            et = MC_EXPR_TYPE_SUB;
        }

        if (et) {
            p->read_pos++;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = et;
            e->add.l     = l;
            e->add.r     = r;

            if (parse_expr10 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr9 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr10 (e, p)) {
        parser_skip_ws (p);

        McExprType et = MC_EXPR_TYPE_INVALID;
        if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "<<", 2)) {
            et = MC_EXPR_TYPE_SHL;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, ">>", 2)) {
            et = MC_EXPR_TYPE_SHR;
        }

        if (et) {
            p->read_pos += 2;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = et;
            e->shl.l     = l;
            e->shl.r     = r;

            if (parse_expr9 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr8 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr9 (e, p)) {
        parser_skip_ws (p);

        McExprType et = MC_EXPR_TYPE_INVALID;
        if (parser_peek (p) == '<') {
            p->read_pos++;
            et = MC_EXPR_TYPE_LT;
        } else if (parser_peek (p) == '>') {
            p->read_pos++;
            et = MC_EXPR_TYPE_GT;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "<=", 2)) {
            p->read_pos += 2;
            et           = MC_EXPR_TYPE_LE;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, ">=", 2)) {
            p->read_pos += 2;
            et           = MC_EXPR_TYPE_GT;
        }
        parser_skip_ws (p);

        if (et) {
            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = et;
            e->le.l      = l;
            e->le.r      = r;

            if (parse_expr8 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr7 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr8 (e, p)) {
        parser_skip_ws (p);

        McExprType et = MC_EXPR_TYPE_INVALID;
        if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "==", 2)) {
            et = MC_EXPR_TYPE_EQ;
        } else if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "!=", 2)) {
            et = MC_EXPR_TYPE_NE;
        }

        if (et) {
            p->read_pos += 2;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = et;
            e->eq.l      = l;
            e->eq.r      = r;

            if (parse_expr7 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr6 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr7 (e, p)) {
        parser_skip_ws (p);

        if (parser_peek (p) == '&') {
            p->read_pos += 1;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_AND;
            e->log_and.l = l;
            e->log_and.r = r;

            if (parse_expr6 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr5 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr6 (e, p)) {
        parser_skip_ws (p);

        if (parser_peek (p) == '^') {
            p->read_pos += 1;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_XOR;
            e->xor.l     = l;
            e->xor.r     = r;

            if (parse_expr5 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr4 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr5 (e, p)) {
        parser_skip_ws (p);

        if (parser_peek (p) == '|') {
            p->read_pos += 1;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_OR;
            e->or.l      = l;
            e->or.r      = r;

            if (parse_expr4 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        return true;
    }

    return false;
}


static inline bool parse_expr3 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr4 (e, p)) {
        parser_skip_ws (p);

        if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "&&", 2)) {
            p->read_pos += 2;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_LOG_AND;
            e->log_and.l = l;
            e->log_and.r = r;

            if (parse_expr3 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // direct pass to expr2
        return true;
    }

    return false;
}


static inline bool parse_expr2 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr3 (e, p)) {
        parser_skip_ws (p);

        if (parser_can_read_n (p, 2) && !strncmp (p->read_pos, "||", 2)) {
            p->read_pos += 2;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_LOG_OR;
            e->log_or.l  = l;
            e->log_or.r  = r;

            if (parse_expr2 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // direct pass to expr2
        return true;
    }

    return false;
}


static inline bool parse_expr1 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr2 (e, p)) {
        parser_skip_ws (p);

        if (parser_peek (p) == '?') {
            p->read_pos++;
            parser_skip_ws (p);

            McExpr* c = expr_create();
            McExpr* t = expr_create();
            McExpr* f = expr_create();

            // we just realized this expression is a ternary operator expr
            *c           = *e;
            e->expr_type = MC_EXPR_TYPE_TERN;
            e->tern.c    = c;
            e->tern.t    = t;
            e->tern.f    = f;

            if (parse_expr1 (t, p)) {
                parser_skip_ws (p);
                p->read_pos++;

                if (parser_peek (p) == ':') {
                    p->read_pos++;
                    parser_skip_ws (p);

                    if (parse_expr1 (f, p)) {
                        return true;
                    }

                    // we expected an expr1, but didn't get one
                    p->read_pos = start_pos;
                    expr_destroy (c);
                    expr_destroy (t);
                    expr_destroy (f);
                    memset (e, 0, sizeof (McExpr));
                    return false;
                }

                // fall through
            }

            // we expected an expr1, but didn't get one
            p->read_pos = start_pos;
            expr_destroy (c);
            expr_destroy (t);
            expr_destroy (f);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // direct pass to expr2
        return true;
    }

    return false;
}


static inline bool parse_expr0 (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr1 (e, p)) {
        parser_skip_ws (p);

        // try to parse assignment expression separately,
        // because we allow multiple assignments
        if (parser_peek (p) == '=') {
            p->read_pos += 1;
            parser_skip_ws (p);

            McExpr* l = expr_create();
            McExpr* r = expr_create();

            *l           = *e;
            e->expr_type = MC_EXPR_TYPE_OR;
            e->assign.l  = l;
            e->assign.r  = r;

            if (parse_expr4 (r, p)) {
                return true;
            }

            p->read_pos = start_pos;
            expr_destroy (l);
            expr_destroy (r);
            memset (e, 0, sizeof (McExpr));
            return false;
        }

        // fall through to parse other operators at expr0 level

        // Operands corresponding to expression type
        struct {
            const char* opnd;
            u8          opnd_len;
            McExprType  expr_type;
        } opnd_type[] = {
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

        // Go through each operand and create the corresponding
        // expression by parsing another right expression
        for (u64 o = 0; o < nopnds; o++) {
            if (parser_can_read_n (p, opnd_type[o].opnd_len) &&
                !strncmp (p->read_pos, opnd_type[o].opnd, opnd_type[o].opnd_len)) {
                // Ignore any whitespace coming after the operator
                p->read_pos += opnd_type[o].opnd_len;
                parser_skip_ws (p);

                // Get the right hand expression
                if (parse_expr1 (r, p)) {
                    McExpr* l = expr_create();
                    *l        = *e;

                    e->expr_type = opnd_type[o].expr_type;
                    e->assign.l  = l;
                    e->assign.r  = r;

                    return true;
                }
            }
        }

        // If none of the operands match, then we make
        // a direct pass to expr1, which was already parsed,
        // and now we just need to return as a successful match
        expr_destroy (r);
        return true;
    }

    return false;
}


static inline bool parse_expr (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    if (parse_expr_list (e, p)) {
        return true;
    } else if (parse_expr_term (e, p)) {
        return true;
    }
    return false;
}


static inline bool parse_expr_list (McExpr* e, McParser* p) {
    if (!e || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    if (parse_expr0 (e, p)) {
        parser_skip_ws (p);

        // if we get a comma, then this is actually a list expression
        if (parser_peek (p) == ',') {
            // create a clone of currently parse expr
            McExpr* xpr = expr_create();
            *xpr        = *e;

            // parse complete list first
            McExprVec list;
            VecInit (&list, NULL, NULL);
            VecPushBack (&list, &xpr);
            while (parser_peek (p) == ',') {
                p->read_pos++;
                parser_skip_ws (p);

                if (parse_expr (e, p)) {
                    // make space for one more expression
                    VecResize (&list, e->list.length + 1);
                    VecLast (&list)  = expr_create();
                    *VecLast (&list) = *e;
                }
            }

            // then change current expr's type to list
            e->expr_type = MC_EXPR_TYPE_LIST;
            e->list      = list;
        }

        return true;
    }

    return false;
}

void print_indent (int indent) {
    for (int i = 0; i < indent; ++i) {
        printf ("  ");
    }
}

void print_expr (McExpr* expr, int indent) {
    if (expr == NULL) {
        print_indent (indent);
        printf ("NULL\n");
        return;
    }

    print_indent (indent);
    switch (expr->expr_type) {
        case MC_EXPR_TYPE_INVALID :
            printf ("Invalid Expression\n");
            break;

        case MC_EXPR_TYPE_ADD :
            printf ("Add: ");
            print_expr (expr->add.l, indent + 1);
            print_expr (expr->add.r, indent + 1);
            break;

        case MC_EXPR_TYPE_SUB :
            printf ("Sub: ");
            print_expr (expr->sub.l, indent + 1);
            print_expr (expr->sub.r, indent + 1);
            break;

        case MC_EXPR_TYPE_MUL :
            printf ("Mul: ");
            print_expr (expr->mul.l, indent + 1);
            print_expr (expr->mul.r, indent + 1);
            break;

        case MC_EXPR_TYPE_DIV :
            printf ("Div: ");
            print_expr (expr->div.l, indent + 1);
            print_expr (expr->div.r, indent + 1);
            break;

        case MC_EXPR_TYPE_AND :
            printf ("And: ");
            print_expr (expr->and.l, indent + 1);
            print_expr (expr->and.r, indent + 1);
            break;

        case MC_EXPR_TYPE_OR :
            printf ("Or: ");
            print_expr (expr->or.l, indent + 1);
            print_expr (expr->or.r, indent + 1);
            break;

        case MC_EXPR_TYPE_XOR :
            printf ("Xor: ");
            print_expr (expr->xor.l, indent + 1);
            print_expr (expr->xor.r, indent + 1);
            break;

        case MC_EXPR_TYPE_MOD :
            printf ("Mod: ");
            print_expr (expr->mod.l, indent + 1);
            print_expr (expr->mod.r, indent + 1);
            break;

        case MC_EXPR_TYPE_SHR :
            printf ("Shr: ");
            print_expr (expr->shr.l, indent + 1);
            print_expr (expr->shr.r, indent + 1);
            break;

        case MC_EXPR_TYPE_SHL :
            printf ("Shl: ");
            print_expr (expr->shl.l, indent + 1);
            print_expr (expr->shl.r, indent + 1);
            break;

        case MC_EXPR_TYPE_LE :
            printf ("Le: ");
            print_expr (expr->le.l, indent + 1);
            print_expr (expr->le.r, indent + 1);
            break;

        case MC_EXPR_TYPE_GE :
            printf ("Ge: ");
            print_expr (expr->ge.l, indent + 1);
            print_expr (expr->ge.r, indent + 1);
            break;

        case MC_EXPR_TYPE_LT :
            printf ("Lt: ");
            print_expr (expr->lt.l, indent + 1);
            print_expr (expr->lt.r, indent + 1);
            break;

        case MC_EXPR_TYPE_GT :
            printf ("Gt: ");
            print_expr (expr->gt.l, indent + 1);
            print_expr (expr->gt.r, indent + 1);
            break;

        case MC_EXPR_TYPE_EQ :
            printf ("Eq: ");
            print_expr (expr->eq.l, indent + 1);
            print_expr (expr->eq.r, indent + 1);
            break;

        case MC_EXPR_TYPE_NE :
            printf ("Ne: ");
            print_expr (expr->ne.l, indent + 1);
            print_expr (expr->ne.r, indent + 1);
            break;

        case MC_EXPR_TYPE_LOG_AND :
            printf ("LogAnd: ");
            print_expr (expr->log_and.l, indent + 1);
            print_expr (expr->log_and.r, indent + 1);
            break;

        case MC_EXPR_TYPE_LOG_OR :
            printf ("LogOr: ");
            print_expr (expr->log_or.l, indent + 1);
            print_expr (expr->log_or.r, indent + 1);
            break;

        case MC_EXPR_TYPE_ASSIGN :
            printf ("Assign: ");
            print_expr (expr->assign.l, indent + 1);
            print_expr (expr->assign.r, indent + 1);
            break;

        case MC_EXPR_TYPE_CALL :
            printf ("Call: ");
            print_expr (expr->call.l, indent + 1);
            print_expr (expr->call.r, indent + 1);
            break;

        case MC_EXPR_TYPE_ARR_SUBSCRIPT :
            printf ("Array Subscript: ");
            print_expr (expr->arr_subscript.l, indent + 1);
            print_expr (expr->arr_subscript.r, indent + 1);
            break;

        case MC_EXPR_TYPE_ACCESS :
            printf ("Access: ");
            print_expr (expr->access.l, indent + 1);
            print_expr (expr->access.r, indent + 1);
            break;

        case MC_EXPR_TYPE_PTR_ACCESS :
            printf ("Pointer Access: ");
            print_expr (expr->ptr_access.l, indent + 1);
            print_expr (expr->ptr_access.r, indent + 1);
            break;

        case MC_EXPR_TYPE_LOG_NOT :
            printf ("LogNot: ");
            print_expr (expr->log_not.e, indent + 1);
            break;

        case MC_EXPR_TYPE_NOT :
            printf ("Not: ");
            print_expr (expr->not.e, indent + 1);
            break;

        case MC_EXPR_TYPE_UN_PLUS :
            printf ("Unary Plus: ");
            print_expr (expr->un_plus.e, indent + 1);
            break;

        case MC_EXPR_TYPE_UN_MINUS :
            printf ("Unary Minus: ");
            print_expr (expr->un_minus.e, indent + 1);
            break;

        case MC_EXPR_TYPE_IN_PARENS :
            printf ("In Parens: ");
            print_expr (expr->in_parens.e, indent + 1);
            break;

        case MC_EXPR_TYPE_ADDR :
            printf ("Address: ");
            print_expr (expr->addr.e, indent + 1);
            break;

        case MC_EXPR_TYPE_DEREF :
            printf ("Dereference: ");
            print_expr (expr->deref.e, indent + 1);
            break;

        case MC_EXPR_TYPE_SIZE_OF :
            printf ("SizeOf: ");
            print_expr (expr->size_of.e, indent + 1);
            break;

        case MC_EXPR_TYPE_ALIGN_OF :
            printf ("AlignOf: ");
            print_expr (expr->align_of.e, indent + 1);
            break;

        case MC_EXPR_TYPE_INC_PFX :
            printf ("Increment Prefix: ");
            print_expr (expr->inc_pfx.e, indent + 1);
            break;

        case MC_EXPR_TYPE_INC_SFX :
            printf ("Increment Suffix: ");
            print_expr (expr->inc_sfx.e, indent + 1);
            break;

        case MC_EXPR_TYPE_DEC_PFX :
            printf ("Decrement Prefix: ");
            print_expr (expr->dec_pfx.e, indent + 1);
            break;

        case MC_EXPR_TYPE_DEC_SFX :
            printf ("Decrement Suffix: ");
            print_expr (expr->dec_sfx.e, indent + 1);
            break;

        case MC_EXPR_TYPE_CAST :
            printf ("Cast to type : ");
            print_expr (expr->cast.e, indent + 1);
            break;

        case MC_EXPR_TYPE_LIST :
            printf ("List:\n");
            for (size_t i = 0; i < expr->list.length; ++i) {
                print_expr (expr->list.data[i], indent + 1);
            }
            break;

        case MC_EXPR_TYPE_TERN :
            printf ("Ternary: ");
            print_expr (expr->tern.c, indent + 1);
            print_expr (expr->tern.t, indent + 1);
            print_expr (expr->tern.f, indent + 1);
            break;

        case MC_EXPR_TYPE_ID :
            printf ("ID: %s\n", expr->id);
            break;

        case MC_EXPR_TYPE_NUM :
            if (expr->num.is_int) {
                printf ("Num: %llu\n", expr->num.i);
            } else {
                printf ("Num: %f\n", expr->num.f);
            }
            break;

        default :
            printf ("Unknown Expression Type: %d\n", expr->expr_type);
            break;
    }
}

bool McParseProgram (McProgram* prog, McParser* p) {
    if (!prog || !p) {
        LOG_ERROR ("invalid arguments.");
        return false;
    }

    const char* start_pos = p->read_pos;
    parser_skip_ws (p);

    while (parser_can_read_n (p, 1)) {
        parser_skip_ws (p);
        McType  type = {0};
        McExpr* e    = expr_create();
        if (parse_basic_type (&type, p)) {
            parser_skip_ws (p);
            puts ("type");
        } else if (parse_expr (e, p)) {
            parser_skip_ws (p);
            print_expr (e, 0);
            expr_destroy (e);
        } else {
            expr_destroy (e);
            break;
        }
        expr_destroy (e);
    }

    if (parser_can_read_n (p, 1)) {
        p->read_pos = start_pos;
        return false;
    }

    return true;
}

McParser* McParserDeinit (McParser* p) {
    if (!p) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    StrDeinit (&p->code);
    memset (p, 0, sizeof (McParser));

    return p;
}


McParser* McParserInit (McParser* p, const char* src_name) {
    if (!p || !src_name) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    memset (p, 0, sizeof (McParser));

    if (!ReadCompleteFile (src_name, (void**)&p->code.data, &p->code.length, &p->code.capacity)) {
        LOG_ERROR ("failed to read complete file \"%s\".", src_name);
        McParserDeinit (p);
        return NULL;
    }

    p->read_pos = p->code.data;

    return p;
}
