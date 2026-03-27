#include "asm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void usage(const char *argv0) {
    fprintf(stderr, "usage: %s <source.poem> [-o <output>]\n", argv0);
}

int main(int argc, char **argv) {
    const char *src = NULL;
    const char *dst = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (++i >= argc) { usage(argv[0]); return 1; }
            dst = argv[i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown flag '%s'\n", argv[i]);
            usage(argv[0]);
            return 1;
        } else {
            if (src) { fprintf(stderr, "error: multiple input files\n"); return 1; }
            src = argv[i];
        }
    }

    if (!src) { usage(argv[0]); return 1; }

    char default_out[512];
    if (!dst) {
        // replace or append .bin
        const char *dot = strrchr(src, '.');
        if (dot) {
            size_t base_len = (size_t)(dot - src);
            if (base_len >= sizeof(default_out) - 5) base_len = sizeof(default_out) - 5;
            memcpy(default_out, src, base_len);
            strcpy(default_out + base_len, ".bin");
        } else {
            snprintf(default_out, sizeof(default_out), "%s.bin", src);
        }
        dst = default_out;
    }

    return assemble_file(src, dst);
}
