#pragma once
#include <stdint.h>

// Binary format: [magic 4B][n_instrs 4B][instrs n*4B][string pool]
// Instruction: [opcode 6b][arg1 13b][arg2 13b]
// BORN uses (arg1<<13)|arg2 as a 26-bit immediate.
// TRAGEDY/EPITAPH: arg1 = byte offset into string pool.

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
#define MAX_LABELS   1024
#define MAX_STRPOOL  65536

#define MASK13 0x1FFF
#define MASK26 0x3FFFFFF

int assemble_file(const char *src, const char *dst);
