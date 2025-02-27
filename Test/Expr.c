#include <Misra/Mc/Parser/ASTNodeTypes.h>
#include <Misra/Std/Log.h>

u64 npass  = 0;
u64 ntotal = 0;

#define CMP_PRECISION 0.0000001

#define FABS(f)        ((f) > 0 ? (f) : -(f))
#define FCMPEQ(f1, f2) (FABS ((f1) - (f2)) <= CMP_PRECISION)

#define TEST_EQ(xpr_str, xpr)                                                                      \
    do {                                                                                           \
        ntotal++;                                                                                  \
        McParser p = {0};                                                                          \
        McParserInitFromZStr (&p, xpr_str);                                                        \
        McExpr e = {0};                                                                            \
        McParseExpr (&e, &p);                                                                      \
        f64 v = 0;                                                                                 \
        if (!FCMPEQ ((v = McExprEval (&e)), (xpr))) {                                              \
            fprintf (                                                                              \
                stderr,                                                                            \
                "[FAIL @ LINE %d] : %s (expected EQ with %lf, got %lf)\n",                         \
                __LINE__,                                                                          \
                xpr_str,                                                                           \
                (f64)(xpr),                                                                        \
                v                                                                                  \
            );                                                                                     \
        } else {                                                                                   \
            npass++;                                                                               \
        }                                                                                          \
        McExprDeinit (&e);                                                                         \
        McParserDeinit (&p);                                                                       \
    } while (0)

#define TEST_NE(xpr_str, xpr)                                                                      \
    do {                                                                                           \
        ntotal++;                                                                                  \
        McParser p = {0};                                                                          \
        McParserInitFromZStr (&p, xpr_str);                                                        \
        McExpr e = {0};                                                                            \
        McParseExpr (&e, &p);                                                                      \
        f64 v = 0;                                                                                 \
        if (FCMPEQ ((v = McExprEval (&e)), (xpr))) {                                               \
            fprintf (                                                                              \
                stderr,                                                                            \
                "[FAIL @ LINE %d] : %s (expected NE with %lf, got %lf)\n",                         \
                __LINE__,                                                                          \
                xpr_str,                                                                           \
                (f64)(xpr),                                                                        \
                v                                                                                  \
            );                                                                                     \
        } else {                                                                                   \
            npass++;                                                                               \
        }                                                                                          \
        McExprDeinit (&e);                                                                         \
        McParserDeinit (&p);                                                                       \
    } while (0)

#define TEST_TYPE_EQ(xpr_str, t)                                                                   \
    do {                                                                                           \
        ntotal++;                                                                                  \
        McParser p = {0};                                                                          \
        McParserInitFromZStr (&p, xpr_str);                                                        \
        McExpr e = {0};                                                                            \
        McParseExpr (&e, &p);                                                                      \
        if (e.expr_type != (t)) {                                                                  \
            fprintf (stderr, "[FAIL_TYPE @ LINE %d] : %s\n", __LINE__, xpr_str);                   \
        } else {                                                                                   \
            npass++;                                                                               \
        }                                                                                          \
        McExprDeinit (&e);                                                                         \
        McParserDeinit (&p);                                                                       \
    } while (0)

#define RESULT()                                                                                   \
    if (ntotal == npass)                                                                           \
        fprintf (stderr, "\nALL PASS! TOTAL = %llu\n", ntotal);                                    \
    else                                                                                           \
        fprintf (stderr, "%llu/%llu PASS\n", npass, ntotal)

int main() {

    // digit, number, variable names
    TEST_EQ ("9", 9);
    TEST_EQ (" 10", 10);
    TEST_EQ (" 141   ", 141);
    TEST_EQ ("9134235", 9134235);

    TEST_TYPE_EQ ("var_name", MC_EXPR_TYPE_ID);
    TEST_TYPE_EQ ("134var_name", MC_EXPR_TYPE_INVALID);

    TEST_EQ ("1 + 2", 1 + 2);
    TEST_EQ ("1 - 2", 1 - 2);
    TEST_EQ ("1 / 2", 1 / 2.f);
    TEST_EQ ("100 / 1000.f", 100 / 1000.f);
    TEST_EQ ("1337.f * 1337.f", 1337.f * 1337.f);
    TEST_EQ ("1 * 2", 1 * 2);
    TEST_EQ ("13 % 5", 13 % 5);
    TEST_EQ ("0xcafebabe << 4", 0xcafebabeULL << 4);
    TEST_EQ ("0xbaadb00b << 13", 0xbaadb00bULL << 13);

    // show result
    RESULT();
}
