# Changelog

## v2.1 R3 - May 2026

### Assembler (hcasm)

#### New Instructions
- **8080/8085 `ldax`**: added `ldax b`, `ldax d`, `ldax bc`, `ldax de`, `ldax [bc]`, `ldax [de]`, `ldax [b]`, `ldax [d]` - Load Accumulator eXtended (complement to existing `stax`), completing the original 8080 instruction set
- **Z80 `EX (SP), IX` / `EX (SP), IY`**: added support for exchanging the stack top with index registers IX and IY, completing the Z80 instruction set

### B Compiler (hcbcomp)

#### Peephole Optimizer Refactoring
- **Per-CPU pattern separation**: CPU-specific peephole patterns moved from `peep.c` into each backend (`gen/z80.c`, `gen/8080.c`, `gen/8086.c`, `gen/8086exe.c`), making each processor implementation fully self-contained
- **New callback architecture**: `gen_peep_replace()` callback declared in `bcomp.h` - each backend provides its own implementation linked at compile time, eliminating the `strcmp(target)` dispatcher
- **Window size fix**: sliding window now tests `w >= 1` (previously started at 2), enabling 1-instruction-to-1-instruction replacements (e.g., `mov ax,0` → `xor ax,ax`)
- **Replacement logic fix**: condition changed from `n < w` to `n <= w` to allow same-count replacements with smaller/faster encoding

#### New Z80 Optimizations
- **Stack cleanup (`inc sp`)**: `ld hl,N; add hl,sp; ld sp,hl` replaced with `inc sp × N` for N ≤ 6 - saves up to 4 bytes and 15 T-states per call
- **Register copy**: `push bc; pop hl` → `ld h,b; ld l,c` - same size, 47% faster
- **Redundant EX elimination**: `ex de,hl; inc sp × N; ex de,hl` → `inc sp × N` - removes unnecessary register swaps around stack adjustments

#### New 8086 Optimizations
- **Zero register**: `mov ax, 0` → `xor ax, ax` - 3 bytes → 2 bytes, ubiquitous in comparison/boolean code

#### New 8086exe Optimizations
- **Zero register**: `mov dx, 0` → `xor dx, dx` - 3 bytes → 2 bytes, very frequent (36+ occurrences in test suite)
- **32-bit negation**: `not ax; not dx; add ax,1; adc dx,0` → `neg dx; neg ax; sbb dx,0` - 8 bytes → 6 bytes

#### New 8080/8085 Optimizations
- **Frame pointer copy**: `push h; pop b` → `mov b,h; mov c,l` - same size, 20 T → 10 T
- **Store immediate**: `push h; lxi h,N; pop d; xchg` → `xchg; lxi h,N; xchg` - 14 bytes → 13 bytes, 40 T → 24 T

#### Math Library
- **8080/8085 runtime library**: `__mul16`, `__div16`, `__mod16` moved from inline code generation to library functions in `libs/b/8080/` and `libs/b/8085/`
- Also added: `__div8`, `__mod8`, `cbw`, comparison helpers (`cmpe8`..`cmpbe16`), shift helpers (`shl8`..`shr16`)
- The B compiler now emits `call __mul16` / `call __div16` / `call __mod16` instead of ~30 bytes of inline code per operation - significant code size reduction for programs using multiple arithmetic operations

### Source Code Documentation

- **Bilingual comments**: All 54 source files across the entire SDK now have function-level comments in both English and Portuguese
- For functions longer than 20 lines, each logical block has inline comments in both languages
- Comment format standardized to `// [English] ...` / `// [Portuguese] ...` across all files

### Test Suite

- **24/24 B language tests passing** across 3 platforms (Z80 8/8, 8080 8/8, 8086 8/8), verified via `msxdosemu` (CP/M) and `emu2` (8086 DOS) emulators
- All test outputs match expected values exactly - confirming optimizations produce correct code

---

## v2.1 R2 - May 2026

### Compiler (hcbcomp)

#### Bug Fixes
- **8080/8085 `gen_cmp_gt`**: removed `xchg` - the GEN_CMPCODE macro already computes `HL - DE` correctly; the extra swap caused `>` to return `right > left` instead of `left > right`
- **8080/8085 local variable offset formula**: fixed from `offset*2 - 2 - nlocals*2` to `-(offset*2 + 4)` - addresses were swapped for functions with multiple locals, causing wrong variable access
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
- **Landing page** (`site-principal/`) - personal developer page linking to `/hcsdk/` documentation
- **Expanded builder docs**: complete `.prj` reference covering all 6 sections and every key
- **Expanded B language reference**: 13 sections (546 lines) covering philosophy, memory model, variables, operators, control flow, functions, arrays, strings, preprocessor, inline functions, B vs C comparison, and 7 complete code examples
- **REX format specification**: header layout, CPU IDs, relocation table, loading algorithm with C pseudocode, hex dump example, format comparison table
- **Librarian docs corrected**: removed fictional `x`/`t`/`d`/`r` commands - hclib only supports `hclib <lib> <objects...>`
- **Installation docs**: added macOS prerequisites (`brew install dpkg llvm mingw-w64 x86_64-unknown-linux-gnu msitools nsis`) and DJGPP setup
- **Downloads page**: links to `/hcsdk/distrosite/` with pre-built packages for all platforms
- All external GitHub links updated to `github.com/humbertocsjr/hcsdkretro`

---

## v2.1 R1 - May 2026

### Compiler (hcbcomp)

#### Right-to-Left Argument Passing
- Arguments are now evaluated and pushed right-to-left directly, eliminating the per-target `gen_reverse_args()` step
- Implemented via two-pass `fseek` technique: Phase 1 records file positions (muted codegen to `/dev/null`), Phase 2 replays in reverse order
- Removed ~180 lines of target-specific reversal assembly across Z80, 8080, 8085, 8086, 8086exe
- Supports unlimited argument count (previously capped at 2-6 depending on target)
- Correctly handles nested function calls and arguments without spaces (`foo(1,2,3)`)
- No ABI change - callee parameter offsets unchanged

#### Peephole Optimizer
- New module `hcbcomp/peep.c` / `hcbcomp/peep.h` with sliding-window pattern matching
- Integrated post-compilation pipeline (compiler writes to temp file, peephole optimizes, writes to real output)
- **Z80 patterns**: local variable load (9→2 instr), local store immediate (12→3 instr), both using direct `[ix+N]` indexed addressing
- **8086 patterns**: LEA+deref chain (3→1 instr), global dereference (3→1 instr), store immediate to local (5→1 instr), push/pop to mov (2→1 instr)
- **8086exe patterns**: 32-bit push/pop pair (4→2 instr), LEA+deref (3→1 instr)
- Typical reduction: ~28% fewer instruction lines (8086), ~32% (Z80)

#### Bug Fixes
- **Z80 comparison operators** (`gen_cmp_gt`, `gen_cmp_lt`, `gen_cmp_le`, `gen_cmp_ge`): added missing `ex de, hl` - operand order was swapped
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

## v2.0 R0 - April 2026

- Initial release: multi-target B compiler (Z80, 8080, 8085, 8086), assembler, linker, librarian, builder, CP/M emulator
- Full B language support with preprocessor
- Runtime libraries for CP/M and MS-DOS
