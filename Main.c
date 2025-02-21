#include <stdio.h>
#include <sys/stat.h>

// ct
#include <Misra/Std/File.h>

int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf (stderr, "usage: scc <src>");
        return 1;
    }

    char*  src_name = argv[1];
    size_t src_size = GetFileSize (src_name);

    printf ("file size = %zu\n", src_size);

    return 0;
}
