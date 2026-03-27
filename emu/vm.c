#include "vm.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *god_name(int g) {
    static const char *names[8] = {
        "ARES", "HERMES", "ATHENA", "ZEUS", "HERA", "POSEIDON", "APOLLO", "HADES"
    };
    return (g >= 0 && g < 8) ? names[g] : "?";
}

static void chorus(const char *fmt, ...) {
    va_list ap;
    fputs("[CHORUS] ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

static void nemesis(VM *vm) {
    vm->hero = 0;
    for (int i = 0; i < 8; i++) {
        vm->gods[i] = 0;
        vm->exiled[i] = 0;
        vm->exile_save[i] = 0;
    }
    free(vm->scroll);
    vm->scroll = NULL;
    vm->scroll_len = 0;
    vm->scroll_cap = 0;
    chorus("NEMESIS descends. All is unmade. The scroll is ash. The gods are silenced.");
}

static int god_exiled(VM *vm, int g) {
    if (g < 0 || g > 7) return 0;
    if (vm->exiled[g]) {
        chorus("%s dwells in exile. No command shall reach them.", god_name(g));
        return 1;
    }
    return 0;
}

static void emit_char(uint32_t v) {
    if (v >= 32 && v <= 126)
        putchar((int)v);
    else
        printf("[%u]", v);
}

static int scroll_push(VM *vm, uint32_t v) {
    if (vm->scroll_len >= vm->scroll_cap) {
        int cap = vm->scroll_cap ? vm->scroll_cap * 2 : 64;
        uint32_t *p = realloc(vm->scroll, (size_t)cap * sizeof(uint32_t));
        if (!p) { fprintf(stderr, "error: scroll memory exhausted\n"); return 1; }
        vm->scroll = p;
        vm->scroll_cap = cap;
    }
    vm->scroll[vm->scroll_len++] = v;
    return 0;
}

static int exec_one(VM *vm) {
    if (vm->pc >= (uint32_t)vm->n_instrs) {
        chorus("The hero walks beyond the last verse. No EPITAPH. Fate is undefined.");
        return VM_TRAP;
    }

    uint32_t word = vm->instrs[vm->pc];
    int op = (int)((word >> 26) & 0x3F);
    int a1 = (int)((word >> 13) & 0x1FFF);
    int a2 = (int)(word & 0x1FFF);

    uint32_t cur_pc = vm->pc;
    vm->pc = cur_pc + 1;

    switch (op) {

    case OP_BORN:
        vm->hero = ((uint32_t)a1 << 13) | (uint32_t)a2;
        chorus("A hero is BORN with the weight of %u.", vm->hero);
        break;

    case OP_SLAY:
        if (god_exiled(vm, a1)) return VM_TRAP;
        if (a1 != GOD_HADES)
            chorus("CHORUS disapproves: %s is not the lord of endings. Yet the deed is done.", god_name(a1));
        emit_char(vm->hero);
        fflush(stdout);
        chorus("%s receives the offering (%u). HERO is silenced.", god_name(a1), vm->hero);
        vm->hero = 0;
        break;

    case OP_GLORY:
        emit_char(vm->hero);
        fflush(stdout);
        chorus("GLORY! The hero's valor (%u) rings out, unchanged.", vm->hero);
        break;

    case OP_QUEST:
        if (god_exiled(vm, a1)) return VM_TRAP;
        if (a1 != GOD_HERMES && a1 != GOD_ARES) {
            chorus("%s has no road to walk. Only HERMES and ARES answer the call of QUEST.", god_name(a1));
            return VM_TRAP;
        }
        {
            uint32_t old = vm->hero;
            vm->hero += vm->gods[a1];
            chorus("%s joins the hero's quest. %u + %u = %u.", god_name(a1), old, vm->gods[a1], vm->hero);
        }
        break;

    case OP_TRIAL:
        if (god_exiled(vm, a1)) return VM_TRAP;
        if (a1 != GOD_HERA) {
            chorus("%s holds no scales. HERA alone may judge.", god_name(a1));
            return VM_TRAP;
        }
        if ((uint32_t)a2 >= (uint32_t)vm->n_instrs) {
            chorus("TRIAL points beyond the edge of the world.");
            return VM_TRAP;
        }
        if (vm->gods[GOD_HERA] == 0) {
            chorus("HERA judges: the scales tip. Fate turns to instruction %u.", (uint32_t)a2);
            vm->pc = (uint32_t)a2;
        } else {
            chorus("HERA judges: balance holds (%u). The hero presses on.", vm->gods[GOD_HERA]);
        }
        break;

    case OP_PROPHECY:
        if ((uint32_t)a1 >= MAX_PROPHECY) {
            chorus("A prophecy beyond the veil of fate. Lost.");
            break;
        }
        vm->prophecy[a1]++;
        chorus("A prophecy is spoken: fate shall one day visit instruction %u.", (uint32_t)a1);
        break;

    case OP_ORACLE:
        if ((uint32_t)a1 >= MAX_PROPHECY || vm->prophecy[a1] == 0) {
            chorus("The oracle speaks without prophecy. The gods are wrathful.");
            return VM_TRAP;
        }
        vm->prophecy[a1]--;
        chorus("The oracle is fulfilled. FATE leaps to instruction %u.", (uint32_t)a1);
        vm->pc = (uint32_t)a1;
        break;

    case OP_HUBRIS: {
        uint64_t r = (uint64_t)vm->hero * 2;
        if (r > UINT32_MAX) {
            chorus("HUBRIS overreaches the heavens. NEMESIS is invoked.");
            nemesis(vm);
        } else {
            uint32_t old = vm->hero;
            vm->hero = (uint32_t)r;
            chorus("HUBRIS: the hero swells with pride. %u -> %u.", old, vm->hero);
        }
        break;
    }

    case OP_EXILE:
        if (god_exiled(vm, a1)) return VM_TRAP;
        vm->exile_save[a1] = vm->gods[a1];
        vm->exiled[a1] = 1;
        vm->gods[a1] = 0;
        chorus("%s is cast into exile, value %u sealed beyond reach.", god_name(a1), vm->exile_save[a1]);
        break;

    case OP_RETURN:
        if (a1 < 0 || a1 > 7 || !vm->exiled[a1]) {
            chorus("%s was never exiled. There is nothing to return.", god_name(a1));
            return VM_TRAP;
        }
        vm->gods[a1] = vm->exile_save[a1];
        vm->exiled[a1] = 0;
        chorus("%s returns from exile bearing value %u.", god_name(a1), vm->gods[a1]);
        break;

    case OP_ODYSSEY:
        if (a2 == 0) {
            chorus("An odyssey of zero iterations. The hero stands motionless.");
            break;
        }
        if (vm->ody_top >= MAX_ODYSSEY) {
            chorus("The odyssey stack overflows. The hero is lost in the labyrinth.");
            return VM_TRAP;
        }
        {
            OdysseyFrame *f = &vm->ody_stack[vm->ody_top++];
            // The ODYSSEY instruction itself is the loop back-edge:
            // when the block's pc falls back to cur_pc, an iteration ends.
            f->return_pc = cur_pc;
            f->block_pc  = (uint32_t)a1;
            f->remaining = a2 - 1;
            chorus("ODYSSEY begins: %d iteration(s), starting at instruction %u.", a2, (uint32_t)a1);
            vm->pc = (uint32_t)a1;
        }
        break;

    case OP_SCROLL:
        if (scroll_push(vm, vm->hero)) return VM_TRAP;
        chorus("APOLLO inscribes %u onto the scroll. Length: %d.", vm->hero, vm->scroll_len);
        break;

    case OP_RECITE:
        chorus("The scroll is recited:");
        for (int i = 0; i < vm->scroll_len; i++)
            emit_char(vm->scroll[i]);
        fflush(stdout);
        break;

    case OP_SET: {
        int g = a1 & 0x7;
        if (god_exiled(vm, g)) return VM_TRAP;
        uint32_t old = vm->gods[g];
        vm->gods[g] = (uint32_t)a2;
        chorus("%s is set: %u -> %u.", god_name(g), old, (uint32_t)a2);
        break;
    }

    case OP_TRAGEDY: {
        const char *msg = (vm->strpool && a1 < vm->strpool_len) ? vm->strpool + a1 : "";
        fprintf(stderr, "[TRAGEDY] %s\n", msg);
        return VM_TRAGEDY;
    }

    case OP_EPITAPH: {
        const char *msg = (vm->strpool && a1 < vm->strpool_len) ? vm->strpool + a1 : "";
        fprintf(stderr, "[CHORUS] %s\n", msg);
        return VM_EPITAPH;
    }

    default:
        chorus("An unknown rite (opcode 0x%02X). The gods recoil.", op);
        return VM_TRAP;
    }

    return VM_CONTINUE;
}

int vm_run(VM *vm) {
    while (1) {
        int ret = exec_one(vm);

        // After each instruction, check if we hit an ODYSSEY return point.
        // Pops completed frames and either re-enters the block or restores return_pc.
        // return_pc is the ODYSSEY instruction itself; when pc lands back on it,
        // the iteration is done. Either restart the block or exit past ODYSSEY.
        while (vm->ody_top > 0) {
            OdysseyFrame *f = &vm->ody_stack[vm->ody_top - 1];
            if (vm->pc != f->return_pc) break;
            if (f->remaining > 0) {
                f->remaining--;
                chorus("ODYSSEY: the hero wanders again to instruction %u.", f->block_pc);
                vm->pc = f->block_pc;
                break;
            } else {
                chorus("ODYSSEY: the wandering is complete.");
                vm->ody_top--;
                vm->pc = f->return_pc + 1; // advance past the ODYSSEY instruction
            }
        }

        if (ret == VM_CONTINUE) continue;
        if (ret == VM_EPITAPH) return 0;
        if (ret == VM_TRAGEDY) return 1;
        return 2;
    }
}

static uint32_t read_u32le(const uint8_t *b) {
    return (uint32_t)b[0]
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
}

int vm_load(VM *vm, const char *path) {
    memset(vm, 0, sizeof(*vm));

    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "error: cannot open '%s'\n", path); return 1; }

    uint8_t hdr[8];
    if (fread(hdr, 1, 8, f) != 8) {
        fprintf(stderr, "error: '%s' is too short\n", path);
        fclose(f); return 1;
    }

    uint32_t magic = read_u32le(hdr);
    uint32_t n     = read_u32le(hdr + 4);

    if (magic != POEM_MAGIC) {
        fprintf(stderr, "error: not a Poem binary (magic 0x%08X)\n", magic);
        fclose(f); return 1;
    }
    if (n > MAX_INSTRS) {
        fprintf(stderr, "error: too many instructions (%u)\n", n);
        fclose(f); return 1;
    }

    if (n > 0) {
        vm->instrs = malloc(n * sizeof(uint32_t));
        if (!vm->instrs) {
            fprintf(stderr, "error: out of memory\n");
            fclose(f); return 1;
        }
        for (uint32_t i = 0; i < n; i++) {
            uint8_t buf[4];
            if (fread(buf, 1, 4, f) != 4) {
                fprintf(stderr, "error: unexpected EOF in instruction stream\n");
                free(vm->instrs); fclose(f); return 1;
            }
            vm->instrs[i] = read_u32le(buf);
        }
    }
    vm->n_instrs = (int)n;

    // String pool: remainder of the file.
    long cur = ftell(f);
    fseek(f, 0, SEEK_END);
    long end = ftell(f);
    int slen = (int)(end - cur);
    if (slen > 0) {
        vm->strpool = malloc((size_t)slen + 1);
        if (!vm->strpool) {
            fprintf(stderr, "error: out of memory for string pool\n");
            free(vm->instrs); fclose(f); return 1;
        }
        fseek(f, cur, SEEK_SET);
        fread(vm->strpool, 1, (size_t)slen, f);
        vm->strpool[slen] = '\0';
        vm->strpool_len = slen;
    }

    fclose(f);
    return 0;
}

void vm_free(VM *vm) {
    free(vm->instrs);
    free(vm->strpool);
    free(vm->scroll);
}
