# HC SDK for Retro Computing

**Version 2.1 R3** - Multi-target cross-development toolchain for retro platforms.

## Overview

HC SDK is a complete development environment for writing software targeting classic 8/16-bit platforms. It includes:

| Tool | Component | Description |
|------|-----------|-------------|
| **hcasm** | Assembler | NASM-syntax multi-target assembler (Z80, 8080, 8085, 8086) |
| **hcbcomp** | B Compiler | B language compiler targeting all four CPUs |
| **hclink** | Linker | Relocatable linker supporting BIN, MZ EXE, and REX formats |
| **hclib** | Librarian | Object file librarian for building `.lib` archives |
| **hcbuild** | Project Builder | Build system using `.prj` project files |
| **msxdosemu** | Emulator | MSX-DOS 1 CP/M emulator for running `.com` executables |

## Objectives

- Provide a **modern development workflow** (cross-compilation from any OS) for retro platforms
- Support **multiple CPU targets** with a single toolchain
- Offer a **high-level language** (B) alongside assembly for faster development
- Generate **small, efficient executables** suitable for memory-constrained retro systems
- Enable **testing without real hardware** via built-in emulation

## Prerequisites for Development

The entire SDK is developed exclusively on **macOS**. Other platforms have not been tested and minimum requirements should be adapted by the user.

### macOS (Development Environment)

Install the minimum requirements:

```sh
# Xcode Command Line Tools (provides clang, make, git)
xcode-select --install

# Homebrew packages for cross-compilation
brew tap messense/macos-cross-toolchains
brew install dpkg llvm mingw-w64 x86_64-unknown-linux-gnu msitools nsis
```

For DOS cross-compilation (DJGPP), download from
[github.com/andrewwutw/build-djgpp/releases](https://github.com/andrewwutw/build-djgpp/releases)
and extract to `/usr/local/djgpp`:

```sh
sudo xattr -r -d com.apple.quarantine /usr/local/djgpp
```

### Build from Source

```sh
git clone https://github.com/humbertocsjr/hcsdkretro.git
cd hcsdkretro
make posix
sudo make install
```

This installs to `/usr/local/bin`. Use `make install PREFIX=/custom/path` to change
the destination.

Platform-specific targets: `make posix` (native), `make linux` (static x86_64),
`make macos` (universal), `make win`/`make win32` (MinGW cross), `make dos` (DJGPP).

## Quick Install (Pre-built Binaries)

Pre-built packages are available at:

**https://humbertocsjr.dev.br/hcsdk/distrosite/**

### macOS / Linux
```sh
# Extract to /usr/local
sudo tar xzf hcsdk-<platform>-2.1r3.tar.gz -C /usr/local
```

### Windows
Run the NSIS installer (`hcsdk-2.1r3-win32.exe`).

### DOS (self-hosted)
Extract `hcsdk-2.1r3-dos.zip` to your DOS drive and add `BIN\` to your `PATH`.

## QuickStart - Hello World in B

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

### Z80 / CP/M
```sh
hcbcomp-z80 -o hello.s hello.b
hcasm-z80 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj libs/z80-cpm-b.lib
msxdosemu hello.com
```

### 8080 / CP/M
```sh
hcbcomp-8080 -o hello.s hello.b
hcasm-8080 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj libs/8080-cpm-b.lib
msxdosemu hello.com
```

### 8086 / MS-DOS
```sh
hcbcomp-8086 -o hello.s hello.b
hcasm-8086 -o hello.obj hello.s
hclink-bin -text 0x100 -o hello.com hello.obj libs/8086-msdos-b.lib
emu2 hello.com
```

### Using the Project Builder
Create `hello.prj`:
```ini
[config]
verbose = yes

[files:z80]
hello.b

[libs]
libs/z80-cpm-b.lib

[link:release]
format = bin
text = 0x100
filename = hello.com
```

Build:
```sh
hcbuild hello.prj make release
```

## Complete Documentation

Full documentation for all tools, the B language reference, REX format specification, and more is available at:

**https://humbertocsjr.dev.br/hcsdk/**

## License

BSD 4-Clause License. See `LICENSE` for details.
