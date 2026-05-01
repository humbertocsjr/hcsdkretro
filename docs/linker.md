# HC Linker — `hclink`

Multi-format linker for object files. Supports flat binary and relocatable executable output.

```sh
hclink-{bin|rex} [options] <object files...>

Options:
  -o <file>          Output file (default: a.bin / a.rex)
  -sym <file>        Symbol table output
  -text <offset>     Text section start address
  -data <offset>     Data section start address
  -bss <offset>      BSS section start address
  -align <offset>    Section alignment
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

### Relocatable Executable (`hclink-rex`)

Produces a relocatable executable with relocation table. See [REX Format](format-rex.md).

```sh
hclink-rex -o program.rex main.obj lib.obj
```

## Object Reordering

The linker automatically reorders objects so the one containing the `_start` or `_main` label is placed first in the output, ensuring the entry point is at the beginning of the text section.

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
