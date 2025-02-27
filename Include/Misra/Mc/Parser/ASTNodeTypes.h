/// file      : misra/mc/astnodetypes.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, Anvie Labs, All rights reserved.
///
/// Parse AST Node types for Modern Compiler.

#ifndef MISRA_MODERN_C_PARSER_AST_NODE_TYPES_H
#define MISRA_MODERN_C_PARSER_AST_NODE_TYPES_H

#include <Misra/Std/Container/Str.h>
#include <Misra/Types.h>

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

typedef struct McParser {
    Str         code;
    const char* read_pos;
} McParser;

///
/// Deinitialize McParser object after using.
///
/// p[out] : Reference to McParser object to be deinitialized.
///
/// SUCCESS : `p`
/// FAILURE : `NULL`
///
McParser* McParserDeinit (McParser* p);

///
/// Initialize a new Modern C Parser object to help read and parse file
/// with given name `src_name`.
///
/// p[out]       : Reference to McParser object to be initialized.
/// src_name[in] : Name of C source code to load and parse.
///
/// SUCCESS : `p`
/// FAILURE : `NULL`
///
McParser* McParserInitFromFile (McParser* p, const char* src_name);

///
/// Initialize a new Modern C Parser object to help read and parse file
/// with given name `src_name`.
///
/// p[out]   : Reference to McParser object to be initialized.
/// code[in] : String to parse as code.
///
/// SUCCESS : `p`
/// FAILURE : `NULL`
///
McParser* McParserInitFromZStr (McParser* p, const char* code);

typedef enum McExprType {
    MC_EXPR_TYPE_INVALID = 0,
    MC_EXPR_TYPE_ADD,
    MC_EXPR_TYPE_SUB,
    MC_EXPR_TYPE_MUL,
    MC_EXPR_TYPE_DIV,
    MC_EXPR_TYPE_AND,
    MC_EXPR_TYPE_OR,
    MC_EXPR_TYPE_XOR,
    MC_EXPR_TYPE_MOD,
    MC_EXPR_TYPE_SHR,
    MC_EXPR_TYPE_SHL,
    MC_EXPR_TYPE_LE,
    MC_EXPR_TYPE_GE,
    MC_EXPR_TYPE_LT,
    MC_EXPR_TYPE_GT,
    MC_EXPR_TYPE_EQ,
    MC_EXPR_TYPE_NE,
    MC_EXPR_TYPE_LOG_AND,
    MC_EXPR_TYPE_LOG_OR,
    MC_EXPR_TYPE_ASSIGN,
    MC_EXPR_TYPE_ADD_ASSIGN,
    MC_EXPR_TYPE_SUB_ASSIGN,
    MC_EXPR_TYPE_MUL_ASSIGN,
    MC_EXPR_TYPE_DIV_ASSIGN,
    MC_EXPR_TYPE_MOD_ASSIGN,
    MC_EXPR_TYPE_AND_ASSIGN,
    MC_EXPR_TYPE_OR_ASSIGN,
    MC_EXPR_TYPE_XOR_ASSIGN,
    MC_EXPR_TYPE_SHR_ASSIGN,
    MC_EXPR_TYPE_SHL_ASSIGN,
    MC_EXPR_TYPE_CALL,
    MC_EXPR_TYPE_ARR_SUBSCRIPT,
    MC_EXPR_TYPE_ACCESS,
    MC_EXPR_TYPE_PTR_ACCESS,
    MC_EXPR_TYPE_LOG_NOT,
    MC_EXPR_TYPE_NOT,
    MC_EXPR_TYPE_UN_PLUS,
    MC_EXPR_TYPE_UN_MINUS,
    MC_EXPR_TYPE_IN_PARENS,
    MC_EXPR_TYPE_ADDR,
    MC_EXPR_TYPE_DEREF,
    MC_EXPR_TYPE_SIZE_OF,
    MC_EXPR_TYPE_ALIGN_OF,
    MC_EXPR_TYPE_INC_PFX,
    MC_EXPR_TYPE_INC_SFX,
    MC_EXPR_TYPE_DEC_PFX,
    MC_EXPR_TYPE_DEC_SFX,
    MC_EXPR_TYPE_CAST,
    MC_EXPR_TYPE_LIST,
    MC_EXPR_TYPE_TERN,
    MC_EXPR_TYPE_ID,
    MC_EXPR_TYPE_NUM,
    MC_EXPR_TYPE_MAX
} McExprType;

///
/// Try to parse type name from the code string in `p`
/// object.
///
/// expr[out] : AST generated after parsing is successful.
/// p[in,out] : McParser object containing code to be parsed.
///
/// SUCCESS : true, type containing parsed typename, p changed.
/// FAILURE : false, type unchanged, p unchanged.
///
bool McParseType (McType* type, McParser* p);

typedef struct McExpr McExpr;
typedef Vec (McExpr*) McExprVec;

struct McExpr {
    McExprType expr_type;

    union {
        struct {
            McExpr* l;
            McExpr* r;
        } add, sub, mul, div, and, or, xor, mod, shr, shl, le, ge, lt, gt, eq, ne, log_and, log_or,
            assign, call, arr_subscript, access, ptr_access;

        struct {
            McExpr* e;
        } log_not, not, un_plus, un_minus, in_parens, addr, deref, size_of, align_of, inc_pfx,
            inc_sfx, dec_pfx, dec_sfx;

        struct {
            McType  type;
            McExpr* e;
        } cast;

        McExprVec list;

        struct {
            McExpr* c;
            McExpr* t;
            McExpr* f;
        } tern;

        Str id;

        struct {
            bool is_int;
            union {
                u64 i;
                f64 f;
            };
        } num;
    };
};

///
/// Traverse the expression tree recursively and evaluate the value.
///
/// expr[in] : Expression tree to be evaluated
///
/// RETURN : evaluated value
///
f64 McExprEval (McExpr* expr);

///
/// Initialize an expression tree as root of tree.
/// 
/// expr[out] : Expression tree to be initialized.
///
/// SUCCESS : expr, initialized expression tree.
/// FAILURE : NULL
///
McExpr* McExprInit(McExpr* expr);

///
/// De-initialize an expression by traversing the tree
/// recursively.
/// 
/// expr[out] : Expression tree to be de-initialized.
///
/// SUCCESS : expr, de-initialized expression tree.
/// FAILURE : NULL
///
McExpr* McExprDeinit(McExpr* expr);

///
/// Try to parse an expression from the code string in `p`
/// object.
///
/// expr[out] : AST generated after parsing is successful.
/// p[in,out] : McParser object containing code to be parsed.
///
/// SUCCESS : true, expr containing parsed expression, p changed.
/// FAILURE : false, expr unchanged, p unchanged.
///
bool McParseExpr (McExpr* expr, McParser* p);

typedef struct McProgram {
} McProgram;

///
/// Try to parse a program from the code string in `p`
/// object.
///
/// prog[out]   : AST generated after parsing is successful.
/// p[in,out] : McParser object containing code to be parsed.
///
/// SUCCESS : true, prog containing parsed program, p changed.
/// FAILURE : false, prog unchanged, p unchanged.
///
bool McParseProgram (McProgram* prog, McParser* p);

#endif // MISRA_MODERN_C_PARSER_AST_NODE_TYPES_H
