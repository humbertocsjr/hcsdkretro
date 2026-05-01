# HC B Compiler — `hcbcomp`

## Overview

HC B Compiler translates the **B programming language** into assembly for multiple retro CPU targets. B is a typeless systems programming language — the predecessor of C — where all data is a machine word (16-bit).

### Supported Targets

| Binary | Target |
|--------|--------|
| `hcbcomp-z80` | Zilog Z80 (CP/M, MSX-DOS) |
| `hcbcomp-8086` | Intel 8086 (MS-DOS) |
| `hcbcomp-8080` | Intel 8080 (CP/M) |
| `hcbcomp-8085` | Intel 8085 (CP/M) |

### Usage

```sh
hcbcomp-{target} [options] <input.b>

Options:
  -o <file>       Output assembly file (default: a.s)
  -I <dir>        Add include directory
  --help, -h      Show help
```

The compiler outputs assembly (`.s`) which is then assembled with `hcasm` and linked with `hclink`.

---

## B Language Reference

### 1. Program Structure

A B program consists of function definitions and global variable declarations at the top level.

```b
/* This is a comment */

/* Global variable (explicit size) */
buf[256];

/* Function definition */
main() {
    auto x;         /* local variable */
    x = 42;
}
```

### 2. Comments

```b
/* C-style block comments */
// C++-style line comments
```

### 3. Data Types

B has no type declarations — all values are 16-bit machine words (integers or addresses).

### 4. Variables

#### Global Variables

```b
var;            /* global word, initialized to 0 */
arr[256];       /* global array of 256 words (512 bytes) */
```

Global variables are accessible from all functions.

#### Local Variables

```b
func() {
    auto x;         /* local word */
    auto y, z;      /* multiple locals */
    auto buf[10];   /* local array */
}
```

Local variables use the `auto` keyword. They are allocated statically per function (no recursion).

#### External Symbols

```b
extrn putchar;      /* external function */
extrn buffer;       /* external variable */
```

The `extrn` keyword declares symbols defined in other modules or libraries.

### 5. Expressions

#### Literals

```b
42          /* decimal */
0xFF        /* hexadecimal */
0777        /* octal */
'A'         /* character constant (65) */
"hello"     /* string (address of null-terminated array) */
```

#### Arithmetic Operators

```b
a + b       /* addition */
a - b       /* subtraction */
a * b       /* multiplication */
a / b       /* division */
a % b       /* modulo */
-a          /* negation */
```

#### Bitwise Operators

```b
a & b       /* bitwise AND */
a | b       /* bitwise OR */
a ^ b       /* bitwise XOR */
~a          /* bitwise NOT */
a << b      /* left shift */
a >> b      /* right shift */
```

#### Comparison Operators

```b
a == b      /* equal */
a != b      /* not equal */
a < b       /* less than */
a > b       /* greater than */
a <= b      /* less or equal */
a >= b      /* greater or equal */
```

Results are 0 (false) or 1 (true).

#### Logical Operators

```b
a && b      /* logical AND (short-circuit) */
a || b      /* logical OR (short-circuit) */
!a          /* logical NOT */
```

#### Assignment

```b
a = b           /* simple assignment */
a += b          /* compound: a = a + b */
a -= b          /* compound: a = a - b */
a *= b          /* compound: a = a * b */
a /= b          /* compound: a = a / b */
a %= b          /* compound: a = a % b */
a &= b          /* compound: a = a & b */
a |= b          /* compound: a = a | b */
a ^= b          /* compound: a = a ^ b */
```

#### Increment/Decrement

```b
++x             /* prefix increment */
--x             /* prefix decrement */
```

#### Array Subscript

```b
arr[i]          /* word at offset i (i * 2 bytes) */
```

Arrays are word-indexed.

#### Pointer Operations

```b
*p              /* dereference (value at address p) */
&x              /* address of variable x */
```

The `&` operator returns the address of a variable. The `*` operator dereferences an address.

#### Function Call

```b
putchar('A');               /* function with one argument */
result = calc(a, b, c);     /* function with multiple arguments */
```

Arguments follow C convention (rightmost pushed first).

#### Comma Operator

```b
a = (x, y);     /* evaluates x, then y; a = y */
```

### 6. Statements

#### Expression Statement

```b
x = y + z;
putchar('A');
;
```

#### Compound Statement

```b
{
    auto x;
    x = 10;
    putchar(x);
}
```

#### If Statement

```b
if (x > 0) {
    putchar('+');
}

if (x == 0) {
    putchar('0');
} else {
    putchar('-');
}
```

#### While Statement

```b
while (i < 10) {
    putchar(arr[i]);
    i = i + 1;
}
```

#### For Statement

```b
for (i = 0; i < 10; i = i + 1) {
    putchar(buf[i]);
}
```

The `for` loop accepts init, test, and increment expressions.

#### Do-While Statement

```b
do {
    c = getchar();
    putchar(c);
} while (c != '\n');
```

#### Return Statement

```b
return 0;
return x + y;
```

Exits the current function with a value.

#### Break Statement

```b
while (i < 100) {
    if (arr[i] == 0) break;
    i = i + 1;
}
```

Exits the innermost loop.

#### Inline Assembly

```b
asm("ld hl, 0x1234");
asm("; this is a comment in assembly");
```

The `asm()` directive inserts raw assembly text directly into the compiler's output.

### 7. Preprocessor Directives

The preprocessor runs before compilation.

#### `#define`

```b
#define MAX 100
#define is_even(x) ((x) % 2 == 0)
```

Simple and function-like macros.

#### `#ifdef` / `#ifndef` / `#else` / `#endif`

```b
#ifdef __Z80__
asm("; compiling for Z80");
#endif

#ifndef NDEBUG
/* debug code */
#endif
```

#### `#include`

```b
#include "stdio.b"
#include <stdlib.b>
```

Searches: source directory, include directories (`-I`), then current directory.

#### Implicit Architecture Macros

| Binary | Macro |
|--------|-------|
| `hcbcomp-z80` | `__Z80__` |
| `hcbcomp-8086` | `__8086__` |
| `hcbcomp-8080` | `__8080__` |
| `hcbcomp-8085` | `__8085__` |

```b
#ifdef __Z80__
asm("ld hl, 0x8000");
#endif
```


---

## Standard Library


### Console I/O

| Function | Description |
|----------|-------------|
| `putchar(c)` | Write character to console |
| `getchar()` | Read character from console (returns -1 on EOF) |
| `gets(s)` | Read line from console into `s` (returns length) |
| `printf(fmt, ...)` | Formatted output |

**printf format specifiers:**

| Specifier | Description |
|-----------|-------------|
| `%d` | Signed decimal |
| `%u` | Unsigned decimal |
| `%x` | Hex lowercase |
| `%s` | String |
| `%c` | Character |
| `%%` | Literal percent |

### File I/O (CP/M / MSX-DOS)

| Function | Description |
|----------|-------------|
| `fopen(name, mode)` | Open file: 0=read, 1=write, 2=append. Returns fd or -1 |
| `fclose(fd)` | Close file. Returns 0 ok, -1 error |
| `fread(fd, buf, nwords)` | Read up to nwords. Returns count |
| `fwrite(fd, buf, nwords)` | Write nwords. Returns count |
| `fseek(fd, offset, whence)` | Seek: 0=SET, 1=CUR, 2=END |
| `ftell(fd)` | Return current position |
| `feof(fd)` | Return EOF flag (non-zero if at end) |
| `fgetc(fd)` | Read one byte (-1 on EOF) |
| `fputc(c, fd)` | Write one byte |
| `fdelete(name)` | Delete file. Returns 0 ok |
| `frename(old, new)` | Rename file. Returns 0 ok |

### String Functions

| Function | Description |
|----------|-------------|
| `strlen(s)` | String length |
| `strcmp(a, b)` | Compare: -1, 0, or 1 |
| `strcpy(dst, src)` | Copy string, returns dst |
| `strcat(dst, src)` | Concatenate, returns dst |

### Standard Library

| Function | Description |
|----------|-------------|
| `abs(n)` | Absolute value |
| `atoi(s)` | ASCII string to integer |
| `itoa(n, s)` | Integer to ASCII string, returns s |
| `memcpy(dst, src, n)` | Copy n words, returns dst |
| `memset(ptr, val, n)` | Set n words to val, returns ptr |

### Inline Functions

These are not function calls — the compiler emits inline code:

| Function | Description | Inline code |
|----------|-------------|-------------|
| `peekb(addr)` | Read 1 byte from addr | `ld l,[hl]; ld h,0` |
| `pokeb(addr, val)` | Write 1 byte to addr | `ld [hl],e` (1 byte) |
| `peekw(addr)` | Read 1 word (2 bytes) | Full word dereference |
| `pokew(addr, val)` | Write 1 word (2 bytes) | Full word store |

---

## Tutorials

### Tutorial 1: Hello World

```b
/* hello.b */
extrn putchar;

main() {
    putchar('H');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar('!');
    putchar(13);
    putchar(10);
}
```

```sh
hcbcomp-z80 -o hello.s hello.b
hcasm-z80 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj bin/b/z80/putchar.obj
```

### Tutorial 2: Using printf

```b
/* printf_test.b */
extrn printf;

main() {
    printf("Hello %s! x = %d\n", "World", 42);
}
```

Compile with the library:

```sh
hcbcomp-z80 -o test.s test.b
hcasm-z80 -o test.obj test.s
hclink-bin -text 0x100 -o test.com test.obj bin/b/z80/printf.obj bin/b/z80/putchar.obj
```

### Tutorial 3: File Copy

```b
/* fcopy.b — copy file */
extrn fopen, fread, fwrite, fclose;

main() {
    auto fd_in, fd_out;
    auto buf[256];
    auto n;

    fd_in = fopen("SOURCE.TXT", 0);
    if (fd_in < 0) return 1;

    fd_out = fopen("DEST.TXT", 1);
    if (fd_out < 0) return 1;

    while (1) {
        n = fread(fd_in, buf, 256);
        if (n == 0) break;
        fwrite(fd_out, buf, n);
    }

    fclose(fd_in);
    fclose(fd_out);
}
```

### Tutorial 4: String Manipulation

```b
/* strdemo.b */
extrn putchar, strlen, strcpy;

main() {
    auto buf[32];

    strcpy(buf, "hello");
    putchar(strlen(buf) + '0');
}
```

### Tutorial 5: Conditional Compilation

```b
/* detect.b — show which CPU we're compiling for */
main() {
#ifdef __Z80__
    asm("ld c, 2");
    asm("ld e, 'Z'");
    asm("call 5");
#endif
#ifdef __8086__
    asm("mov ah, 2");
    asm("mov dl, '8'");
    asm("int 0x21");
#endif
}
```

---

## Compilation Pipeline

```
source.b
    │
    ▼
[Preprocessor]  →  #define, #ifdef, #include expansion
    │
    ▼
[Compiler]      →  Generates .s assembly
    │
    ▼
[hcasm]         →  Assembles to .obj
    │
    ▼
[hclink]        →  Links to .com/.bin/.rex
```

## Calling Convention

| Aspect | Convention |
|--------|-----------|
| **Parameter passing** | Stack, right-to-left (C convention) |
| **Stack cleanup** | Caller cleans (`add sp, n*2` after call) |

## Library Structure

```
libs/b/
├── strlen.b, strcmp.b, ...    /* individual string functions */
├── gets.b, printf.b           /* I/O functions */
├── abs.b, atoi.b, ...         /* stdlib functions */
├── z80/
│   ├── putchar.s, getchar.s   /* CP/M BDOS wrappers */
│   ├── bdos.s                 /* BDOS primitive */
│   ├── fopen.b, fclose.b, ... /* File I/O (B source) */
│   ├── cpm_byte.b, cpm_slot.b, cpm_name.b  /* helpers */
│       └── start.s                /* _start with argc/argv */
├── 8086/
│   ├── putchar.s, ...         /* MS-DOS INT 21h wrappers */
│   └── dos_file.s             /* File I/O (partial) */
```

Libraries are built with `hclib` and stored in `bin/b/<cpu>/`.
