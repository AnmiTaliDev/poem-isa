# Poem — ISA of the Heroic Epic

Poem is an intentionally inconvenient instruction set architecture styled as an ancient epic poem. Every program is an epic. The processor is fate. The programmer is the author — but not a god.

**License:** GNU GPL 3.0

---

## Architecture at a glance

| Entity   | Description |
|----------|-------------|
| HERO     | The sole 32-bit accumulator. Every instruction revolves around it. |
| FATE     | Program counter. Not directly readable or writable. |
| GODS     | Eight named registers, each with a strict character — wrong usage traps. |
| CHORUS   | Narrates every instruction to stderr. Cannot be silenced. |
| SCROLL   | Unbounded linear memory. Written by `SCROLL`, read by `RECITE`. |

See [`spec/isa.md`](spec/isa.md) for the full specification.

---

## Building

```sh
make -C as
make -C emu
```

Produces `as/poem-as` and `emu/poem-emu`.

---

## Usage

**Assemble:**
```sh
./as/poem-as source.asm -o program.bin
```
Default output name: `source.bin`.

**Run:**
```sh
./emu/poem-emu program.bin
```

CHORUS commentary goes to stderr. Program output goes to stdout.

```sh
./emu/poem-emu program.bin 2>/dev/null   # stdout only
./emu/poem-emu program.bin 2>&1          # both streams
```

**Exit codes:**

| Code | Meaning |
|------|---------|
| 0    | `EPITAPH` reached — the epic is complete |
| 1    | `TRAGEDY` — the hero fell by the author's hand |
| 2    | Trap — a god was offended, a prophecy was broken, or the epic ran off the edge |

---

## Example

```
; example/hello_world.asm
BORN 72      ; 'H'
SLAY HADES
BORN 101     ; 'e'
SLAY HADES
...
EPITAPH "The world was named. The gods were satisfied."
```

```sh
./as/poem-as example/hello_world.asm -o /tmp/hw.bin
./emu/poem-emu /tmp/hw.bin 2>/dev/null
# Hello, World!
```

---

## Binary format

All produced binaries share the same flat layout:

```
[magic: 0x504F454D — 4 bytes, LE]
[instruction count  — 4 bytes, LE]
[instructions       — count × 4 bytes, LE]
[string pool        — null-terminated strings, back to back]
```

Each instruction is 32 bits: `[opcode : 6][arg1 : 13][arg2 : 13]`.

---

## Gods and their characters

| God      | Allowed operations |
|----------|--------------------|
| ARES     | `QUEST` |
| HERMES   | `SET`, `QUEST` |
| ATHENA   | Logical comparisons *(no instructions in v0.1)* |
| ZEUS     | Multiply and divide *(no instructions in v0.1)* |
| HERA     | `TRIAL` |
| POSEIDON | Bit shifts *(no instructions in v0.1)* |
| APOLLO   | `SCROLL`, `RECITE` (implicit) |
| HADES    | `SLAY` |

Using a god outside its character is a trap (exit 2).
`SLAY` accepts any god, but CHORUS will voice its disapproval.
