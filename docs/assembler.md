# HC Assembler — `hcasm`

Multi-target assembler inspired by NASM's source code format. Supports four CPU architectures.

```sh

Options:
  --help, -h    Show help
```

## Source Code Format

```asm
label: mnemonic arg1, arg2 ; comment
```

### Labels

```asm
global _start       ; export for linker
_start:             ; global label
.loop:              ; sub-label (prefixed with parent label)
    jmp .loop       ; references _start.loop
```

### Sections

```asm
section text        ; code section
section data        ; initialized data section
section bss         ; uninitialized data section (zero-filled)
```

Section order in output: text → data → bss

### Data Directives

```asm
db 0x12, 0x34      ; define bytes
dw 0x1234          ; define words (2 bytes, little-endian)
ds 10              ; reserve 10 bytes (or resb/rb)
resw 5             ; reserve 5 words (10 bytes)
```

### Constants (EQU)

```asm
SCREEN equ 0xC000
msg: db "Hello"
msg_len equ $-msg   ; current position
```

### Expressions

```asm
mov ax, 0x1234     ; hexadecimal
mov ax, 1234       ; decimal
mov ax, 0b1010     ; binary
mov ax, 0o777      ; octal
mov ax, label      ; address of label
mov ax, label+5    ; arithmetic
```

## Z80 Support

```asm
; Standard Z80 syntax with []
ld a, [hl]
ld a, [bc]
ld hl, [0x1234]
ld [ix+5], a
ld a, [ix-4]       ; negative offsets supported
```

## 8080/8085 Support

```asm
; Intel 8080 syntax
mov a, b           ; 8-bit register move
lxi h, 0x1234      ; load immediate 16-bit
dad d              ; HL += DE
stax b             ; store A at [BC]

; Alternative formats
stax bc            ; same as stax b
stax [bc]          ; NASM-like format
stax [b]           ; old-school format

; Memory operations
mov a, m           ; A = [HL]
mov a, [hl]        ; same
mov a, [m]         ; same
```

## 8086 Support

```asm
; Standard x86 syntax with prefixes
rep movsb
mov word [0x1234], 0x5678
call near label
jmp far segment:offset

; Conditional jumps with distance
je near label      ; force near jump
je short label     ; force short jump
```
