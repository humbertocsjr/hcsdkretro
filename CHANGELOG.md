# Changelog

## v2.1 R2 — May 2026

### Compiler (hcbcomp)

#### Bug Fixes
- **8080/8085 `gen_cmp_gt`**: removed `xchg` — the GEN_CMPCODE macro already computes `HL - DE` correctly; the extra swap caused `>` to return `right > left` instead of `left > right`
- **8080/8085 local variable offset formula**: fixed from `offset*2 - 2 - nlocals*2` to `-(offset*2 + 4)` — addresses were swapped for functions with multiple locals, causing wrong variable access
- **8080/8085 `gen_mul`, `gen_div`, `gen_mod`**: added `push b`/`pop b` to preserve BC register (frame pointer), preventing corruption of subsequent local variable accesses

### Builder (hcbuild)

- **Removed RetroLang references**: `retrolang-` compiler invocation replaced with `hcbcomp-`; `[retrolang:include_path]` section renamed to `[blang:include_path]`
- **Removed `.rl` extension**: only `.b`/`.B` accepted as B language source files
- **Added `-I` include path support**: the `[blang:include_path]` section and CLI `-I` flags are now passed to the B compiler for `.b` files
- Help text updated: "RetroLang" → "B compiler"

### Test Suite

- **42/42 tests passing** across 5 platforms (Z80 11/11, 8080 10/10, 8085 1/1, 8086 11/11, 8086exe 9/9)
- Tests run on emu2 for 8086 and 8086exe targets
- Test runner fixed for temp file naming with special characters

### Documentation

- **New HTML3 website** (`site/`) with frameset layout, sidebar navigation, and 20+ content pages in retro 90s style
- **Landing page** (`site-principal/`) — personal developer page linking to `/hcsdk/` documentation
- **Expanded builder docs**: complete `.prj` reference covering all 6 sections and every key
- **Expanded B language reference**: 13 sections (546 lines) covering philosophy, memory model, variables, operators, control flow, functions, arrays, strings, preprocessor, inline functions, B vs C comparison, and 7 complete code examples
- **REX format specification**: header layout, CPU IDs, relocation table, loading algorithm with C pseudocode, hex dump example, format comparison table
- **Librarian docs corrected**: removed fictional `x`/`t`/`d`/`r` commands — hclib only supports `hclib <lib> <objects...>`
- **Installation docs**: added macOS prerequisites (`brew install dpkg llvm mingw-w64 x86_64-unknown-linux-gnu msitools nsis`) and DJGPP setup
- **Downloads page**: links to `/hcsdk/distrosite/` with pre-built packages for all platforms
- All external GitHub links updated to `github.com/humbertocsjr/hcsdkretro`

---

## v2.1 R1 — May 2026

### Compiler (hcbcomp)

#### Right-to-Left Argument Passing
- Arguments are now evaluated and pushed right-to-left directly, eliminating the per-target `gen_reverse_args()` step
- Implemented via two-pass `fseek` technique: Phase 1 records file positions (muted codegen to `/dev/null`), Phase 2 replays in reverse order
- Removed ~180 lines of target-specific reversal assembly across Z80, 8080, 8085, 8086, 8086exe
- Supports unlimited argument count (previously capped at 2-6 depending on target)
- Correctly handles nested function calls and arguments without spaces (`foo(1,2,3)`)
- No ABI change — callee parameter offsets unchanged

#### Peephole Optimizer
- New module `hcbcomp/peep.c` / `hcbcomp/peep.h` with sliding-window pattern matching
- Integrated post-compilation pipeline (compiler writes to temp file, peephole optimizes, writes to real output)
- **Z80 patterns**: local variable load (9→2 instr), local store immediate (12→3 instr), both using direct `[ix+N]` indexed addressing
- **8086 patterns**: LEA+deref chain (3→1 instr), global dereference (3→1 instr), store immediate to local (5→1 instr), push/pop to mov (2→1 instr)
- **8086exe patterns**: 32-bit push/pop pair (4→2 instr), LEA+deref (3→1 instr)
- Typical reduction: ~28% fewer instruction lines (8086), ~32% (Z80)

#### Bug Fixes
- **Z80 comparison operators** (`gen_cmp_gt`, `gen_cmp_lt`, `gen_cmp_le`, `gen_cmp_ge`): added missing `ex de, hl` — operand order was swapped
- **Z80 `gen_cmp_le`**: fixed inverted branch logic where `<=` returned false for true
- **8080/8085 subtraction** (`gen_sub`): removed erroneous `xchg` that swapped operands

### Assembler (hcasm)

- **8086 `emit_mrm_complete`**: fixed `|` (bitwise OR) → `&&` (logical AND) that caused segfaults
- **8086 immediate-to-memory**: added support for `mov word [mem], imm` and `mov word [reg+disp], imm`
- Added `lex_get_fp()`, `lex_sync()`, `lex_get_ch()`, `lex_set_ch()` to lexer

### Builder (hcbuild)

- Added `[files:8086exe]` as supported platform selector

### Linker (hclink)

- MZ EXE format (`hclink-mz`): segment constants `__data_seg_delta__`, `__bss_seg_delta__`, `__stack_top__`

### Hello-B Sample

- Fixed 8086 EXE project (`[files:8086]` → `[files:8086exe]`) for far-pointer library compatibility

---

## v2.0 R0 — April 2026

- Initial release: multi-target B compiler (Z80, 8080, 8085, 8086), assembler, linker, librarian, builder, CP/M emulator
- Full B language support with preprocessor
- Runtime libraries for CP/M and MS-DOS
