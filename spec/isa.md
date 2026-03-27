# Poem ISA Specification

## 1. Philosophy

Every program in Poem is an epic. The processor is fate. The programmer is the author — but not a god. The gods already exist, and they have their own character.

A program that does not end with `EPITAPH` is an unfinished tale. Behavior is undefined.

---

## 2. Architectural entities

### 2.1 HERO — the protagonist

The sole accumulator. A 32-bit unsigned integer. Every program has exactly one HERO — it is not a register, it is *destiny*.

- Read and written by most instructions.
- Cannot be renamed.
- Overflow of HERO through `HUBRIS` automatically invokes `NEMESIS`.

### 2.2 FATE — program counter

Points to the currently executing instruction. Not directly readable or writable — controlled only through `ORACLE`, `TRIAL`, and `ODYSSEY`. The value of FATE can only be observed indirectly through `PROPHECY`.

### 2.3 CHORUS — the chorus

A commentary stream. After every instruction, CHORUS narrates what happened to `stderr`. CHORUS cannot be disabled — it always watches.

CHORUS does not affect execution. It simply *knows*.

### 2.4 GODS — register gods

Eight registers, each a god with character. Each god accepts only certain operations. Violating a god's character is a trap.

| Name     | Character                  | Permitted operations                        |
|----------|----------------------------|---------------------------------------------|
| ARES     | God of war                 | `QUEST`                                     |
| HERMES   | The messenger              | `SET`, `QUEST`                              |
| ATHENA   | Goddess of wisdom          | Logical comparisons *(no instructions in v0.1)* |
| ZEUS     | The thunderer              | Multiply and divide *(no instructions in v0.1)* |
| HERA     | Queen of the gods          | `TRIAL`                                     |
| POSEIDON | Lord of the seas           | Bit shifts *(no instructions in v0.1)*      |
| APOLLO   | God of light               | `SCROLL`, `RECITE` (implicit)               |
| HADES    | Lord of the dead           | `SLAY`                                      |

A god in exile (`EXILE`) is unavailable for any operations until `RETURN`.

### 2.5 SCROLL — the memory scroll

Linear memory. Indexed from 0. Read and written only through APOLLO's domain. The scroll has no fixed size — it grows as values are written.

---

## 3. Type system

The ISA knows one type: **32-bit unsigned integer**. Everything else is interpretation. `GLORY` and `SLAY` interpret values as ASCII characters. The programmer decides what numbers mean.

---

## 4. Instruction set

### 4.1 Initialization and termination

---

#### `BORN <val>`

Set HERO to `val`.

```
BORN 72        ; HERO = 72
```

- `val` — integer, 0 to 67108863 (26-bit limit imposed by the instruction encoding)
- Previous value of HERO is destroyed without warning
- CHORUS proclaims the birth

---

#### `EPITAPH "<text>"`

End the program. The last instruction of any correct epic.

```
EPITAPH "The hero fell. The gods were pleased."
```

- Text is written to `stderr` as CHORUS's final word
- A program without `EPITAPH` ends with undefined behavior
- No instructions execute after `EPITAPH`

---

### 4.2 Output

---

#### `SLAY <god>`

Proclaim victory over the god: output HERO as an ASCII character to `stdout`, then set HERO to 0.

```
BORN 72
SLAY HADES     ; prints 'H', HERO = 0
```

- The only god permitted for `SLAY` without reproach is HADES (his character)
- For all other gods, `SLAY` executes but CHORUS expresses disapproval
- If HERO < 32 or HERO > 126 — prints `[N]` instead of the character

---

#### `GLORY`

Output HERO as an ASCII character to `stdout` without resetting it.

```
GLORY          ; prints the character, HERO unchanged
```

- Does not reset HERO
- CHORUS delivers a triumphant line
- Recommended as the last output before `EPITAPH`

---

#### `RECITE`

Read the entire scroll aloud — output its contents to `stdout` as a string of characters.

```
RECITE         ; outputs everything in the scroll
```

- The scroll is not cleared by `RECITE`
- Values < 32 or > 126 are printed as `[N]`

---

### 4.3 Arithmetic and logic

---

#### `QUEST <god>`

HERO += god. The hero sets out on a journey with the god.

```
SET HERMES, 10
QUEST HERMES   ; HERO = HERO + 10
```

- Permitted only with HERMES and ARES
- Any other god — trap

---

#### `HUBRIS`

HERO *= 2. Pride doubles the hero's strength.

```
HUBRIS         ; HERO = HERO * 2
```

- If the result exceeds 32 bits — `NEMESIS` is invoked automatically
- `NEMESIS` resets HERO and all gods to 0, clears the scroll
- `NEMESIS` is not a separate instruction — it is punishment, not a command

---

#### `SET <god>, <val>`

Assign a value to a god directly.

```
SET ARES, 255
```

- `val` — integer, 0 to 8191 (13-bit limit imposed by the instruction encoding)
- A god in exile cannot be `SET`

---

### 4.4 Control flow

---

#### `PROPHECY <addr>`

Declare that a jump to `addr` will occur in the future. Without `PROPHECY`, `ORACLE` is impossible.

```
PROPHECY loop_start
```

- `addr` — numeric address or label
- One `PROPHECY` permits exactly one `ORACLE` to that address
- After `ORACLE` consumes it, the prophecy is fulfilled

---

#### `ORACLE <addr>`

Unconditional jump to `addr`. Requires a preceding `PROPHECY`.

```
PROPHECY loop_start
...
ORACLE loop_start   ; jump permitted
```

- Without a preceding `PROPHECY` — trap: «The gods gave no warning»
- FATE is set to `addr`

---

#### `TRIAL <god>, <addr>`

Conditional jump: if god == 0, jump to `addr`.

```
TRIAL HERA, end    ; if HERA == 0 — jump to end
```

- Permitted only with HERA (her character is judgment)
- If the condition is not met — execution continues

---

#### `ODYSSEY <addr>, <n>`

Execute the block starting at `addr` exactly `n` times. The ODYSSEY instruction itself marks the end of the loop body — when the block's execution naturally returns to the ODYSSEY instruction, one iteration is complete.

```
ODYSSEY print_char, 5
```

- `addr` — label or address of the block's first instruction
- `n` — integer > 0
- The block runs from `addr` until it falls through to the ODYSSEY instruction
- After all `n` iterations, execution continues at the instruction after ODYSSEY
- Internal jumps within the block that escape back to ODYSSEY also end the iteration

---

### 4.5 Exile of gods

---

#### `EXILE <god>`

Exile the god: save its value and make it inaccessible.

```
EXILE ARES     ; ARES inaccessible until RETURN
```

- Any access to an exiled god — trap
- The value is preserved in a place inaccessible to the programmer directly

---

#### `RETURN <god>`

Restore an exiled god with its former value.

```
RETURN ARES    ; ARES accessible again
```

- If the god was not in exile — trap: «Nothing to return»

---

### 4.6 The scroll

---

#### `SCROLL`

Write the current value of HERO to the end of the scroll.

```
BORN 65
SCROLL         ; scroll: [..., 65]
```

- Only APOLLO governs the scroll; `SCROLL` is his sole write instruction
- The scroll grows without bound

---

### 4.7 Catastrophe

---

#### `TRAGEDY "<text>"`

Deliberate program crash with a message.

```
TRAGEDY "The hero met something incomprehensible."
```

- Writes the message to `stderr`, exits with code 1
- State is not preserved

---

## 5. Labels

Labels are declared as `name:` on a line by themselves and may be used in place of numeric addresses in `ORACLE`, `TRIAL`, `PROPHECY`, and `ODYSSEY`.

```
loop_start:
    BORN 65
    SLAY HADES
```

Label names: Latin letters, digits, underscores. Case-insensitive.

---

## 6. Comments

A comment starts with `;` and continues to the end of the line.

```
BORN 72    ; this is 'H'
```

---

## 7. Instruction encoding

All instructions are fixed-length — **32 bits**.

```
[ OPCODE : 6 bits ][ ARG1 : 13 bits ][ ARG2 : 13 bits ]
```

| Instruction | Opcode | ARG1         | ARG2         |
|-------------|--------|--------------|--------------|
| BORN        | 000001 | val[25:13]   | val[12:0]    |
| SLAY        | 000010 | god          | —            |
| GLORY       | 000011 | —            | —            |
| QUEST       | 000100 | god          | —            |
| TRIAL       | 000101 | god          | addr         |
| PROPHECY    | 000110 | addr         | —            |
| ORACLE      | 000111 | addr         | —            |
| HUBRIS      | 001000 | —            | —            |
| EXILE       | 001001 | god          | —            |
| RETURN      | 001010 | god          | —            |
| ODYSSEY     | 001011 | addr         | n            |
| SCROLL      | 001100 | —            | —            |
| RECITE      | 001101 | —            | —            |
| SET         | 001110 | god          | val          |
| TRAGEDY     | 001111 | string offset| —            |
| EPITAPH     | 111111 | string offset| —            |

Gods are encoded as a 3-bit index in ARG1:
`ARES=000, HERMES=001, ATHENA=010, ZEUS=011, HERA=100, POSEIDON=101, APOLLO=110, HADES=111`

`BORN` encodes a 26-bit immediate split across both argument fields: `value = (ARG1 << 13) | ARG2`.

`TRAGEDY` and `EPITAPH` store a byte offset into the string pool (see binary format).

---

## 8. Binary format

Produced by the assembler, consumed by the emulator:

```
[ magic: 0x504F454D — 4 bytes, little-endian ]
[ instruction count — 4 bytes, little-endian ]
[ instructions      — count × 4 bytes, little-endian ]
[ string pool       — null-terminated strings, back to back ]
```

The string pool holds all string literals from `TRAGEDY` and `EPITAPH`. String offsets in those instructions are byte offsets from the start of the pool.

Addresses in all instructions are instruction indices (0-based), not byte offsets.

---

## 9. Error behavior

| Situation | Name | Behavior |
|---|---|---|
| HUBRIS overflow | NEMESIS | Reset all state, continue execution |
| ORACLE without PROPHECY | Wrath of the gods | Trap, exit code 2 |
| Access to exiled god | Exile | Trap, exit code 2 |
| God character violation | Offense | Trap, exit code 2 |
| Missing EPITAPH | Unfinished epic | Undefined behavior |
| TRAGEDY | Tragedy | Exit code 1 |

---

## 10. Design notes

- Poem is intentionally inconvenient. This is not a bug.
- The absence of direct access to FATE makes debugging an act of faith.
- God characters mean that every task requires the right god — you cannot use ARES for comparison, even if you want to.
- CHORUS is always right. If CHORUS says something went wrong, it did.
- ODYSSEY marks the end of its own loop body. The block is the code between its start label and the ODYSSEY instruction.
