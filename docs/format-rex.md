# Relocatable Executable Format (REX)

The REX format is a relocatable executable format with relocation table support.

## File Layout

```
+-------------------+
| Header (16 bytes) |
+-------------------+
| Text section      |
+-------------------+
| Data section      |
+-------------------+
| BSS section       |
+-------------------+
| Relocation table  |
+-------------------+
```

## Header

| Offset | Size | Description |
|--------|------|-------------|
| 0x0000 | 2 | Magic: `"HC"` |
| 0x0002 | 2 | Text section size |
| 0x0004 | 2 | Data section size |
| 0x0006 | 2 | BSS section size |
| 0x0008 | 2 | `_start` offset (relative to text section base) |
| 0x000A | 2 | Relocation table size (number of entries) |
| 0x000C | 3 | Reserved |
| 0x000F | 1 | CPU ID |

## CPU IDs

| Value | CPU |
|-------|-----|
| 0xF0 | Intel 8080 |
| 0xF1 | Intel 8085 |
| 0xF2 | Zilog Z80 |
| 0xF3 | Intel 8086 |

## Relocation Table

Each entry is a 2-byte offset relative to the text section base. During loading:

1. Read each relocation entry (2-byte offset)
2. Read the word (2 bytes) at that offset in the loaded program
3. Add the text section base address to that word

## Loading Process

```c
// 1. Read header (16 bytes)
// 2. Allocate text_size + data_size + bss_size bytes
// 3. Copy text and data segments
// 4. Zero-fill BSS
// 5. Process relocation table:
//    for each entry:
//        word at [text_base + entry] += text_base
// 6. Execute at text_base + _start_offset
```
