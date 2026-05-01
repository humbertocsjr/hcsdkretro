# Quick Start Guide

## Hello World in Assembly (Z80 / CP/M)

Create `hello.s`:

```asm
section data
msg: db "Hello World!$"

section text
global _start
_start:
    ld de, msg
    ld c, 9          ; BDOS print string
    call 5
    ld c, 0          ; BDOS exit
    call 5
```

Assemble, link, and run:

```sh
hcasm-z80 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj
msxdosemu hello.com
```

## Hello World in B (Z80 / CP/M)

Create `hello.b`:

```b
extrn putchar;

main() {
    putchar('H');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar('!');
}
```

Compile and link:

```sh
hcbcomp-z80 -o hello.s hello.b
hcasm-z80 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj libs/z80-cpm-b.lib
```

## Using the Project Builder

Create `hello.prj`:

```ini
[config]
verbose = yes
sdk_path = ./bin

[files:z80]
hello.s

[link:release]
format = bin
text = 0x100
filename = hello.com
```

Build:

```sh
hcbuild hello.prj make release
```

## B Language Project

Create `calc.prj`:

```ini
[config]

[files:z80]
calc.b

[link:release]
format = bin
text = 0x100
filename = calc.com
```

```b
/* calc.b — simple calculator */
extrn putchar;

main() {
    putchar('4');
    putchar('2');
}
```

```sh
hcbuild calc.prj make release
```

## Next Steps

- Read the [HC B Compiler](bcompiler.md) documentation for the complete B language reference
- See [HC Assembler](assembler.md) for assembly syntax
- Check the [samples/](../samples/) directory for example projects
