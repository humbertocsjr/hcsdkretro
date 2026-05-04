# HC Builder — `hcbuild`

Project build system that reads `.prj` configuration files and orchestrates compilation, assembly, and linking. The project file path is used as the base directory for all relative source file paths.

## Command Line

```sh
hcbuild [options] <project.prj> <command> [config]
```

| Argument | Description |
|----------|-------------|
| `<project.prj>` | Path to the project configuration file (required) |
| `<command>` | `make` to build, `clean` to remove output files (required) |
| `[config]` | Link configuration name. Default: `release`. Allows multiple `[link:NAME]` sections. |

### Options

| Option | Description |
|--------|-------------|
| `--sdk-path <dir>` | Base directory for all tools. Overrides `HC_SDK_PATH` and `sdk_path` in `[config]`. |
| `--asm-path <dir>` | Assembler directory (defaults to `sdk_path`). |
| `--link-path <dir>` | Linker directory (defaults to `sdk_path`). |
| `--lib-path <dir>` | Librarian directory (defaults to `sdk_path`). |
| `--output-dir <dir>` | Output directory. Overrides `HC_OUTPUT_DIR` and `output_dir` in `[config]`. |
| `-I <dir>` | Add include path (for B language `#include`). Repeatable. |
| `--verbose, -v` | Print each tool invocation before executing. |
| `--dump` | Generate `.dump` assembly listing files alongside `.obj` files. |
| `--keep` | Keep intermediate `.__s` files (normally deleted after assembly). |
| `--help, -h` | Show help message. |

### Environment Variables

| Variable | Description |
|----------|-------------|
| `HC_SDK_PATH` | Base directory for all tools. Overridden by `--sdk-path`. |
| `HC_OUTPUT_DIR` | Output directory. Overridden by `--output-dir`. |

## Build Order

When `make` is executed, hcbuild processes the project in this order:

1. Collect objects from `[lib:start]` and `[libs:start]` (linked first)
2. Compile and assemble source files from all `[files:*]` sections
3. Collect objects from `[lib]` and `[libs]` (linked after user objects)
4. Invoke the linker with all collected objects

---

## .prj File Format — Complete Reference

Lines starting with `;` or `#` are comments. Section headers use `[name:subsection]`. Keys can have values after `=`, or stand alone (no `=` needed for source file entries). Values may be quoted with double quotes.

### 1. `[config]` — Global Settings

Optional section. All keys are optional.

| Key | Values | Description |
|-----|--------|-------------|
| `sdk_path` | Path string | Base directory where `hcbcomp-*`, `hcasm-*`, `hclink-*`, and `hclib` are located. |
| `output_dir` | Path string | Directory for all output files. If not set, output goes next to the `.prj` file. |
| `verbose` | `yes`, `true`, `1`, `enable` | Print each tool command before executing. |
| `dump` | `yes`, `true`, `1`, `enable` | Generate `.dump` assembly listing files. |
| `keep` | `yes`, `true`, `1`, `enable` | Keep intermediate `.__s` files after assembly. |

```ini
[config]
sdk_path = ../../bin
verbose = yes
keep = true
```

### 2. `[files:ARCH]` — Source Files

Lists source files for a specific target architecture. Each line is a filename (no `=` value). Multiple `[files:ARCH]` sections can coexist in one project.

| Architecture | Compiler | Assembler | Use Case |
|-------------|----------|-----------|----------|
| `z80` | `hcbcomp-z80` | `hcasm-z80` | Z80 / CP/M, MSX-DOS |
| `8086` | `hcbcomp-8086` | `hcasm-8086` | 8086 / MS-DOS COM (near pointers) |
| `8086exe` | `hcbcomp-8086exe` | `hcasm-8086` | 8086 / MS-DOS EXE (far pointers, MZ format) |
| `8080` | `hcbcomp-8080` | `hcasm-8080` | 8080 / CP/M |
| `8085` | `hcbcomp-8085` | `hcasm-8085` | 8085 / CP/M |

> **Note:** `[files:8086exe]` compiles with `hcbcomp-8086exe` and assembles with `hcasm-8086`. Use this when linking with the EXE runtime library (`8086-msdos-exe-b.lib`) for MZ-format executables with independent segments.

#### Supported Source File Extensions

| Extension | Step 1 | Step 2 | Notes |
|-----------|--------|--------|-------|
| `.b`, `.B` | Compile to `.__s` | Assemble to `.obj` | B language source. Uses `hcbcomp-ARCH`. |
| `.s`, `.S` | — | Assemble to `.obj` | Assembly source. Uses `hcasm-ARCH`. |
| `.obj` | Passed directly to linker | — | Pre-compiled object file. |

```ini
[files:z80]
main.b
utils.s
startup.obj

[files:8086]
portable.b
```

### 3. `[libs]` and `[lib]` — Library Files

Libraries and objects linked **after** all source files have been compiled and assembled. Both section names are equivalent.

```ini
[libs]
../../libs/z80-cpm-b.lib
extra/mylib.lib
```

### 4. `[libs:start]` and `[lib:start]` — Startup Objects

Libraries and objects linked **before** user source files. Use this for startup code (`_start`). The linker ensures the object containing `_start` is placed first.

```ini
[libs:start]
start.obj
crt.lib
```

### 5. `[link:NAME]` — Link Configuration

Defines how to produce the final executable. `NAME` is the configuration name (e.g., `release`, `debug`). Pass the name as the third argument to hcbuild. If only one `[link:*]` section exists, it is used regardless of name.

| Key | Default | Description |
|-----|---------|-------------|
| `format` | `bin` | Output format: `bin` = flat binary (COM/ROM), `mz` = MZ EXE (MS-DOS segmented), `rex` = REX relocatable, `lib` = librarian output (produces `.lib` using hclib). |
| `filename` | `a.out` | Output file name. Relative paths are resolved from the `output_dir` or project directory. |
| `text` | (none) | Text section start address (hex or decimal). Typical CP/M .COM: `0x100`. |
| `data` | (none) | Data section start address. If omitted, data follows immediately after text. |
| `bss` | (none) | BSS section start address. If omitted, BSS follows immediately after data. |
| `align` | (none) | Section alignment in bytes. Ensures each section starts at a multiple of this value. |
| `stack` | 4096 | Stack size in bytes. Only meaningful for `format = mz`. The stack is placed at the end of the BSS section. |
| `symbols` | (none) | If set, generates a symbol table file listing all labels with addresses. |

```ini
; CP/M .COM target
[link:release]
format = bin
text = 0x100
filename = program.com
symbols = program.sym

; MS-DOS .EXE target
[link:msdos]
format = mz
stack = 1024
filename = program.exe

; ROM image target
[link:rom]
format = bin
text = 0x4000
bss = 0xC000
filename = firmware.bin
```

### 6. `[blang:include_path]` — B Compiler Include Paths

Additional include directories passed to the B compiler via `-I`. Each line is a path. These paths are used for `#include` resolution in `.b` source files.

```ini
[blang:include_path]
../include
../lib/common
```

## Examples

### Minimal Assembly Project

```ini
[files:z80]
main.s

[link:release]
format = bin
text = 0x100
filename = program.com
```

### B Language Project with Libraries

```ini
[config]
sdk_path = ./bin

[files:z80]
main.b

[libs]
bin/b/z80/cpm-b.lib

[link:release]
format = bin
text = 0x100
filename = program.com
```

### Multi-File Project

```ini
[config]
verbose = yes

[files:z80]
startup.b
main.b
utils.b

[libs]
mylib.lib

[link:release]
format = bin
text = 0x100
filename = app.com
symbols = app.sym
```

### 8086 MS-DOS EXE Project

```ini
[config]
sdk_path = ./bin

[files:8086exe]
main.b

[libs]
libs/8086-msdos-exe-b.lib

[link:release]
format = mz
stack = 1024
filename = program.exe
```

### Multi-Configuration Project

```ini
; Build both CP/M and MS-DOS versions from the same project
[config]
sdk_path = ../../bin
verbose = yes

[files:z80]
main.b
sound.b
gfx.b

[files:8086exe]
main.b
sound.b
gfx.b
dos_wrapper.s

[libs]
../../libs/z80-cpm-b.lib

[link:cpm]
format = bin
text = 0x100
filename = GAME.COM

[link:dos]
format = mz
stack = 4096
filename = GAME.EXE
```

```sh
hcbuild game.prj make cpm      # build CP/M version
hcbuild game.prj make dos      # build MS-DOS EXE version
hcbuild game.prj clean         # remove all outputs
```
