# HC Linker — `hclink`

Multi-format linker for object files. Supports flat binary, MZ EXE, and relocatable executable output.

```sh
hclink-{bin|mz|rex} [options] <object files...>

Options:
  -o <file>          Output file (default: a.bin / a.exe / a.rex)
  -sym <file>        Symbol table output
  -text <offset>     Text section start address
  -data <offset>     Data section start address
  -bss <offset>      BSS section start address
  -align <offset>    Section alignment
  -stack <size>      Stack size in bytes (MZ format, default: 4096)
  -multicpu          Allow mixed CPU object files
  -v                 Verbose
  --help, -h         Help
```

## Output Formats

### Flat Binary (`hclink-bin`)

Produces a contiguous binary suitable for ROMs, CP/M .COM files, or raw memory images.

```sh
# CP/M .COM file
hclink-bin -text 0x100 -o program.com start.obj program.obj lib.lib

# Generic binary
hclink-bin -o firmware.bin code.obj data.obj

# ROM image
hclink-bin -text 0x4000 -bss 0xC000 -o game.rom game.obj
```

### MS-DOS MZ EXE (`hclink-mz`)

Produces an MS-DOS MZ-format executable with independent segments (TEXT, DATA, BSS). Far pointers are used for inter-segment references.

```sh
# MS-DOS .EXE file
hclink-mz -stack 1024 -o program.exe start.obj main.obj lib.lib
```

Segment deltas are automatically computed and exposed as constants:
- `__data_seg_delta__` — paragraphs from TEXT to DATA
- `__bss_seg_delta__` — paragraphs from TEXT to BSS
- `__stack_top__` — top of stack (BSS + stack size)

### Relocatable Executable (`hclink-rex`)

Produces a relocatable executable with relocation table. See [REX Format](format-rex.md).

```sh
hclink-rex -o program.rex main.obj lib.obj
```

## Object Reordering

The linker automatically reorders objects so the one containing the `_start` label is placed first in the output, ensuring the entry point is at the beginning of the text section.

## Dead Code Elimination

Unreferenced object files are automatically excluded from the link, reducing output size.

## Symbol Table

```sh
hclink-bin -sym symbols.txt -o program.com program.obj
```

Output format:

```
$0000 _start [GLOBAL LABEL object.obj]
$0100 data [GLOBAL LABEL object.obj]
```
