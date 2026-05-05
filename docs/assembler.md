# HC Assembler — `hcasm`

Multi-target assembler supporting **Z80**, **8080/8085**, and **8086** instruction sets. Produces relocatable object files (`.obj`) for linking with `hclink`.

## Usage

```sh
hcasm-{target} [options] <input.s>

Options:
  -o <file>       Output object file (default: a.obj)
  -dump <file>    Generate assembly listing dump
  --help, -h      Show help
```

Targets: `hcasm-z80`, `hcasm-8080`, `hcasm-8085`, `hcasm-8086`

---

## Source Code Format

```
label: mnemonic arg1, arg2 ; comment
```

- **Labels**: end with `:` and must start at column 1 (no leading whitespace)
- **Instructions**: must be indented with a tab or spaces
- **Comments**: start with `;` and run to end of line
- **Case-insensitive**: all mnemonics, registers, and directives

### Labels

```asm
global _start       ; export symbol for linker
extern printf       ; import symbol from another module

_start:             ; global label (colon at end)
.loop:              ; sub-label (prefixed as _start.loop)
    jp .loop        ; references _start.loop
```

- **Global labels** (`global`) are exported for linking
- **External labels** (`extern`) are resolved at link time
- **Sub-labels** (starting with `.`) are local to the most recent global label
- Sub-label `.loop` after `_start:` becomes `_start.loop` internally

### Sections

```asm
section text        ; executable code
section data        ; initialized data (db, dw, dd)
section bss         ; uninitialized data (ds, rb, resb, rw, resw)
```

- Section order in output: **text → data → bss**
- Multiple `section text` blocks are merged into one contiguous text section
- `section bss` data is allocated at link time and zero-filled at load

---

## Data Directives

### `db` — Define Bytes

```asm
db 0x12, 0x34, 0x56     ; raw byte values
db "Hello", 0            ; string followed by null
db 'A', 10, 13           ; mix of char, decimal, hex
```

### `dw` — Define Words (16-bit, little-endian)

```asm
dw 0x1234                ; single word
dw 100, 200, 300         ; multiple words
dw label                 ; address of label (relocatable)
dw "AB"                  ; two ASCII bytes in one word
```

### `dd` — Define Double Words (32-bit, little-endian)

```asm
dd 0x12345678            ; 32-bit value
dd label                 ; 32-bit address (segment:offset for 8086)
```

### `ds` / `rb` / `resb` — Reserve Bytes

```asm
ds 256                   ; reserve 256 bytes (all synonyms)
rb 128
resb 64
```

All three are equivalent. The space is allocated in the output section (BSS space is not stored in the object file, only its size).

### `rw` / `resw` — Reserve Words

```asm
rw 128                   ; reserve 128 words (256 bytes)
resw 64                  ; same
```

### `equ` — Constants

```asm
BUFSIZ equ 512
SCREEN equ 0xC000
msg: db "Hello"
msg_len equ $ - msg      ; $ = current position
```

Constants are evaluated at assembly time. They can be used in expressions anywhere a number is expected.

### `times` — Repeat

```asm
times 256 db 0           ; emit 256 zero bytes
times 10 nop             ; emit 10 NOP instructions
```

The `times` prefix repeats the following directive or instruction N times.

---

## Expressions and Operators

### Number Formats

| Format | Example | Notes |
|--------|---------|-------|
| Decimal | `42`, `-1` | Standard |
| Hexadecimal | `0xFF`, `0x1A2B` | `0x` prefix |
| Hexadecimal | `0FFh`, `1A2Bh` | Trailing `h` (must start with digit) |
| Binary | `0b1010` | `0b` prefix |
| Octal | `0o777`, `0777` | `0o` prefix or leading `0` with octal digits |

### Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `label + 4` |
| `-` | Subtraction | `label - 2` |
| `*` | Multiplication | `100 * 4` |
| `/` | Division | `1024 / 16` |
| `%` | Modulo | `size % 256` |
| `$` | Current position | `$-msg` (size of data so far) |
| `<` or `LOBYTE` | Low byte of value | `mov al, <label` |
| `>` or `HIBYTE` | High byte of value | `mov ah, >label` |

Expressions can use labels defined elsewhere in the same file. Labels from other modules must be declared `extern`.

### Immediate Operator `#`

The `#` prefix forces a value to be treated as an immediate operand:

```asm
mov ax, #42         ; explicit immediate
mov ax, #label      ; address as immediate
```

---

## Platform-Specific Syntax

### Z80

#### Register Names
`a`, `b`, `c`, `d`, `e`, `h`, `l` (8-bit) — `af`, `bc`, `de`, `hl`, `ix`, `iy`, `sp` (16-bit)

#### Memory Addressing
The Z80 uses **square brackets** `[]` for memory operands:

```asm
ld a, [hl]              ; A = *HL
ld a, [bc]              ; A = *BC
ld a, [de]              ; A = *DE
ld hl, [0x1234]         ; HL = *0x1234 (direct address)
ld a, [ix+5]            ; A = *(IX + 5)
ld a, [ix-4]            ; negative offsets supported
ld [ix+0], 42          ; immediate to memory
```

#### Key Differences from 8080/8085
- `[]` brackets for all memory access (8080 uses `mov a,m` or no brackets)
- Index registers `ix`, `iy` with signed displacement
- No `( )` parentheses — always `[]`
- Extended instruction set: `sbc hl,de`, `adc hl,bc`, `ex de,hl`, `exx`, `djnz`

#### Complete Example
```asm
section text
global _start
_start:
    ld hl, message
    ld c, 9             ; BDOS print string
    call 5
    ld c, 0             ; BDOS exit
    call 5

section data
message: db "Hello, World!$"
```

### 8080/8085

#### Register Names
`a`, `b`, `c`, `d`, `e`, `h`, `l` (8-bit) — `bc`, `de`, `hl`, `sp`, `psw` (16-bit)

#### Memory Addressing
The 8080/8085 supports **three memory access styles** — all equivalent:

```asm
mov a, m            ; traditional Intel: A = *HL
mov a, [hl]         ; NASM-style: same
mov a, [m]          ; explicit memory: same

stax b              ; *BC = A
stax bc             ; same (register pair name)
stax [bc]           ; NASM-style
stax [b]            ; old-school single register

ldax b              ; A = *BC
ldax bc             ; same (register pair name)
ldax [bc]           ; NASM-style
ldax [b]            ; old-school single register
```

**Important**: `m` and `[hl]` are equivalent on 8080/8085 — both access memory at `HL`.

#### 16-bit Loads

```asm
lxi h, 0x1234       ; HL = 0x1234
lhld 0x1234         ; HL = *0x1234 (load from direct address)
shld 0x1234         ; *0x1234 = HL (store to direct address)
```

#### 8085 Extensions

The 8085 adds `rim` (read interrupt mask) and `sim` (set interrupt mask). The assembler accepts these only for the 8085 target.

#### Complete Example
```asm
section text
global _start
_start:
    lxi h, message
    mvi c, 9
    call 5
    mvi c, 0
    call 5

section data
message: db "Hello, World!$"
```

### 8086

#### Register Names
**8-bit**: `al`, `cl`, `dl`, `bl`, `ah`, `ch`, `dh`, `bh`
**16-bit**: `ax`, `cx`, `dx`, `bx`, `sp`, `bp`, `si`, `di`
**Segment**: `es`, `cs`, `ss`, `ds`

#### Memory Addressing

All memory operands use **square brackets** `[]`:

```asm
mov ax, [bx]            ; register indirect
mov ax, [bp-2]          ; based with displacement
mov ax, [bx+si]         ; based + indexed
mov ax, [bp+si+4]       ; based + indexed + displacement
mov ax, [0x1234]        ; direct address
mov word [0x1234], 0x5678  ; immediate to memory
```

#### Size Prefixes

When the operand size is ambiguous, use size prefixes:

```asm
mov word [bx], 1        ; store 16-bit value
mov byte [bx], 0        ; store 8-bit value
inc word [si]           ; increment 16-bit memory
inc byte [di]           ; increment 8-bit memory
```

Also: `dword` for 32-bit, `qword` for 64-bit (where applicable).

#### Distance Prefixes

Override the default jump/call distance:

```asm
je near label           ; force 16-bit relative offset
je short label          ; force 8-bit relative offset
call far segment:offset ; far call (32-bit address)
jmp far seg:off         ; far jump
retf                    ; far return
```

#### Segment Overrides

```asm
es mov ax, [bx]         ; read from ES:BX instead of DS:BX
ss mov [bp-2], ax       ; write to SS:BP-2
cs mov ax, [si]         ; read from CS:SI
ds mov [di], ax         ; explicit DS (default, usually omitted)
```

#### Repeat Prefixes

```asm
rep movsb               ; repeat MOVSB CX times
rep stosw               ; repeat STOSW CX times
repe cmpsb              ; repeat while equal
repne scasb             ; repeat while not equal
```

Rep prefixes apply to the **next instruction only**.

#### Complete Example
```asm
section text
global _start
_start:
    mov ah, 9
    mov dx, message
    int 0x21
    mov ah, 0x4C
    int 0x21

section data
message: db "Hello, World!$"
```

---

## Target Comparison Quick Reference

| Feature | Z80 | 8080/8085 | 8086 |
|---------|-----|-----------|------|
| Memory access | `[reg]`, `[ix+N]` | `m`, `[hl]`, `[bc]` | `[reg]`, `[reg+N]`, `[reg+reg]` |
| Indexed addressing | `ix`/`iy` with `[ix+d]` | None | `[bp+N]`, `[bx+si]`, etc |
| Immediate to memory | `ld [ix+N], imm` | Not native | `mov word [mem], imm` |
| Segment registers | N/A | N/A | `es`, `cs`, `ss`, `ds` |
| Size prefixes | N/A | N/A | `byte`, `word`, `dword` |
| Distance prefixes | N/A | `jmp`/`call` only | `short`, `near`, `far` |
| Repeat prefixes | N/A | N/A | `rep`, `repe`, `repne` |
| 16-bit immediate | `ld hl, imm` | `lxi h, imm` | `mov ax, imm` |
| Direct address load | `ld hl, (addr)` | `lhld addr` | `mov ax, [addr]` |
| Return value reg | `hl` | `hl` | `ax` |

---

## Complete Example — All Targets

### Z80 CP/M "Hello World"

```asm
section text
global _start
_start:
    ld de, msg
    ld c, 9
    call 5
    ret

section data
msg: db "Hello!$"
```

### 8080 CP/M "Hello World"

```asm
section text
global _start
_start:
    lxi d, msg
    mvi c, 9
    call 5
    ret

section data
msg: db "Hello!$"
```

### 8086 MS-DOS "Hello World"

```asm
section text
global _start
_start:
    mov ah, 9
    mov dx, msg
    int 0x21
    mov ah, 0x4C
    int 0x21

section data
msg: db "Hello!$"
```
