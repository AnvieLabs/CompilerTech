#include <Misra/Mc/Parser/ASTNodeTypes.h>
#include <Misra/Std/Log.h>

int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf (stderr, "usage: mcc <src>\n");
        return 1;
    }

    const char* src_name = argv[1];

    McParser parser = {0};
    if (!McParserInitFromFile (&parser, src_name)) {
        LOG_ERROR ("failed to init parser.");
        return 1;
    }

    McProgram program = {};

    printf ("program = %b\n", McParseProgram (&program, &parser));

    McParserDeinit (&parser);

    return 0;
}
