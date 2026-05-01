# HC Builder — `hcbuild`

Project build system that reads `.prj` configuration files and orchestrates compilation, assembly, and linking.

```sh
hcbuild [options] <project.prj> <command> [config]

Commands:
  make              Build project
  clean             Remove output files

Options:
  --sdk-path <dir>       SDK tools directory
  --asm-path <dir>       Assembler directory
  --link-path <dir>      Linker directory
  --lib-path <dir>       Librarian directory
  --output-dir <dir>     Output directory
  -I <dir>               Include directory (RetroLang/B files)
  --verbose, -v          Verbose output
  --dump                 Generate assembly dumps
  --keep                 Keep intermediate files
  --help, -h             Help

Environment variables:
  HC_SDK_PATH            SDK tools directory
  HC_OUTPUT_DIR          Output directory
```

## Project File Format (`.prj`)

```ini
[config]               ; optional
dump = yes
verbose = yes
sdk_path = ./bin       ; path to hcasm, hclink, etc.

[files:z80]            ; source files for architecture
main.s                 ; assembly files
module.b               ; B language files (compiled to .s, then assembled)

[libs:start]           ; startup objects (linked first)
start.obj

[libs]                 ; libraries and objects
mylib.lib

[link:release]         ; link configuration
format = bin
filename = program.com
text = 0x100
data = 0x2000
bss = 0xC000
align = 0x100
symbols = program.sym  ; optional symbol table output
```

### Supported File Extensions

| Extension | Action |
|-----------|--------|
| `.s`, `.S` | Assemble with `hcasm-<arch>` |
| `.b`, `.B` | Compile with `hcbcomp-<arch>` → `.s` → `hcasm-<arch>` |
| `.obj` | Link directly |
| `.lib` | Link library |
| `.rl`, `.RL` | Compile with `retrolang-<arch>` → `.__s` → `hcasm-<arch>` |

### Architecture Selectors

```ini
[files:z80]         ; Z80 source files
[files:8086]        ; 8086 source files
[files:8080]        ; 8080 source files
[files:8085]        ; 8085 source files
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
