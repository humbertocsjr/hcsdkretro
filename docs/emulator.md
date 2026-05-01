# MSX-DOS 1.0 / CP/M 2.2 Emulator — `msxdosemu`

Standalone emulator for MSX-DOS 1.0 (CP/M 2.2 compatible) with embedded debugger.

```sh
msxdosemu [options] <program.com> [arguments...]

Options:
  -diska <path>    Set disk A directory
  -debug           Debug mode (stops at first breakpoint)
  -step            Step-by-step debug mode
  -skip <addr>     Set initial breakpoint address
  --help, -h       Help
```

## Usage Examples

```sh
# Run program
msxdosemu cat.com test.txt

# Run with disk image from directory
msxdosemu -diska other/dir/ cat.com test.txt

# Debug mode
msxdosemu -debug program.com

# Step mode
msxdosemu -step program.com

# Set initial breakpoint
msxdosemu -skip 0x103 program.com
```

## Supported BDOS Calls

### MSX-DOS 1.0 / CP/M 2.2 (FCB-based)

- 0x00–0x2F: Standard CP/M functions
- 0x32: Get MSX-DOS version
- 0x62: Terminate with error code
- 0x6F: Get MSX-DOS version number

### MSX-DOS 2.0 (Extended)

- 0x40–0x47: File handle API
- 0x48–0x49: Get/Set default drive
- 0x4A–0x4D: Directory operations
- 0x4E–0x4F: Find First/Next
- 0x50–0x53: File date/time/size
- 0x54: Get disk free space
- 0x56: IOCTL
- 0x5A: Rename
- 0x5B–0x5C: File info, attributes
- 0x5D–0x5F: Environment variables
- 0x63: MSX-DOS 2.0 version

## Key Shortcuts

| Key | Action |
|-----|--------|
| F12 | Toggle debug mode |
| CTRL+F12 | Toggle step mode |
| ALT+F12 | Reset |
