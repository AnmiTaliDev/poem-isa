#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    VM vm;
    if (vm_load(&vm, argv[1]) != 0) return 1;
    int ret = vm_run(&vm);
    vm_free(&vm);
    return ret;
}
