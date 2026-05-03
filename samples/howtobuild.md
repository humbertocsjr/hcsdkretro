```sh
cd hello-b
hcbuild z80cpm.prj make release       # Z80 CP/M
hcbuild 8080cpm.prj make release      # 8080 CP/M
hcbuild 8085cpm.prj make release      # 8085 CP/M
hcbuild 86msdos.prj make release      # 8086 MS-DOS COM
hcbuild 86msdos-exe.prj make release  # 8086 MS-DOS EXE
```

## Building the samples

All hello-b samples use the B compiler with their respective platform selectors:

| Project file | Platform | Compiler | Output |
|-------------|----------|----------|--------|
| `z80cpm.prj` | `[files:z80]` | `hcbcomp-z80` | `Z80CPM.COM` |
| `8080cpm.prj` | `[files:8080]` | `hcbcomp-8080` | `8080CPM.COM` |
| `8085cpm.prj` | `[files:8085]` | `hcbcomp-8085` | `8085CPM.COM` |
| `86msdos.prj` | `[files:8086]` | `hcbcomp-8086` | `86MSDOS.COM` |
| `86msdos-exe.prj` | `[files:8086exe]` | `hcbcomp-8086exe` | `86MSDOS.EXE` |

> **Note:** The EXE project uses `[files:8086exe]` for far-pointer calling convention compatibility with the `8086-msdos-exe-b.lib` library. The compiler is `hcbcomp-8086exe` and the assembler is `hcasm-8086`.