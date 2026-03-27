#pragma once
#include <stdint.h>

#define POEM_MAGIC   0x504F454D

#define OP_BORN      0x01
#define OP_SLAY      0x02
#define OP_GLORY     0x03
#define OP_QUEST     0x04
#define OP_TRIAL     0x05
#define OP_PROPHECY  0x06
#define OP_ORACLE    0x07
#define OP_HUBRIS    0x08
#define OP_EXILE     0x09
#define OP_RETURN    0x0A
#define OP_ODYSSEY   0x0B
#define OP_SCROLL    0x0C
#define OP_RECITE    0x0D
#define OP_SET       0x0E
#define OP_TRAGEDY   0x0F
#define OP_EPITAPH   0x3F

#define GOD_ARES     0
#define GOD_HERMES   1
#define GOD_ATHENA   2
#define GOD_ZEUS     3
#define GOD_HERA     4
#define GOD_POSEIDON 5
#define GOD_APOLLO   6
#define GOD_HADES    7

#define MAX_INSTRS   65536
#define MAX_PROPHECY 8192
#define MAX_ODYSSEY  256

// An active ODYSSEY call frame on the loop stack.
typedef struct {
    uint32_t return_pc; // instruction index to return to after block finishes
    uint32_t block_pc;  // start of the block being repeated
    int remaining;      // iterations still to run after the current one
} OdysseyFrame;

typedef struct {
    uint32_t hero;

    uint32_t gods[8];
    int      exiled[8];
    uint32_t exile_save[8];

    uint32_t *scroll;
    int       scroll_len;
    int       scroll_cap;

    uint32_t *instrs;
    int       n_instrs;

    char *strpool;
    int   strpool_len;

    int prophecy[MAX_PROPHECY]; // pending prophecy count per instruction address

    OdysseyFrame ody_stack[MAX_ODYSSEY];
    int          ody_top;

    uint32_t pc;
} VM;

#define VM_CONTINUE 0
#define VM_EPITAPH  1 // clean exit
#define VM_TRAGEDY  2 // exit code 1
#define VM_TRAP     3 // exit code 2

int  vm_load(VM *vm, const char *path);
int  vm_run(VM *vm);
void vm_free(VM *vm);
