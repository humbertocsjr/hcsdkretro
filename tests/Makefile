# HC SDK Test Suite
# Run with: make test           (compile + assemble + link all tests)
#           make test_emu       (run emulator-based tests)

SDK    = ../bin
SDKABS = $(shell cd $(dir $(lastword $(MAKEFILE_LIST))) && pwd)/../bin
LIBS   = $(SDKABS)/../libs

ASM_Z80   = $(SDK)/hcasm-z80
ASM_8080  = $(SDK)/hcasm-8080
ASM_8085  = $(SDK)/hcasm-8085
ASM_8086  = $(SDK)/hcasm-8086
COM_Z80   = $(SDK)/hcbcomp-z80
COM_8080  = $(SDK)/hcbcomp-8080
COM_8085  = $(SDK)/hcbcomp-8085
COM_8086  = $(SDK)/hcbcomp-8086
COM_8086E = $(SDK)/hcbcomp-8086exe
LINK_BIN  = $(SDK)/hclink-bin
LINK_MZ   = $(SDK)/hclink-mz
EMU_CPM   = $(SDK)/msxdosemu
EMU_86    = /usr/local/bin/emu2
BUILD     = $(SDK)/hcbuild

# ── Assembler tests (assemble only, check exit code) ──────────

ASM_TESTS_Z80   = $(patsubst asm/z80/%.s,  out/asm_z80_%,  $(wildcard asm/z80/*.s))
ASM_TESTS_8080  = $(patsubst asm/8080/%.s, out/asm_8080_%, $(wildcard asm/8080/*.s))
ASM_TESTS_8085  = $(patsubst asm/8085/%.s, out/asm_8085_%, $(wildcard asm/8085/*.s))
ASM_TESTS_8086  = $(patsubst asm/8086/%.s, out/asm_8086_%, $(wildcard asm/8086/*.s))
ASM_ALL = $(ASM_TESTS_Z80) $(ASM_TESTS_8080) $(ASM_TESTS_8085) $(ASM_TESTS_8086)

# ── B language tests (compile + run on emulator, check output) ──

B_Z80_SRCS   = $(wildcard b/*.b)
B_Z80_TESTS  = $(patsubst b/%.b, out/b_z80_%, $(B_Z80_SRCS))
B_8080_TESTS = $(patsubst b/%.b, out/b_8080_%, $(B_Z80_SRCS))
B_8086_TESTS = $(patsubst b/%.b, out/b_8086_%, $(B_Z80_SRCS))
B_ALL = $(B_Z80_TESTS) $(B_8080_TESTS) $(B_8086_TESTS)

# ── Phony targets ──────────────────────────────────────────────

.PHONY: all test test_asm test_b test_emu clean

all: test

test: test_asm test_b
	@echo ""
	@echo "=========================================="
	@echo "  ALL TESTS PASSED"
	@echo "=========================================="

test_asm: $(ASM_ALL)
	@echo "[ASM] All assembler tests passed"

test_b: $(B_ALL)
	@echo "[B]   All B language tests passed"

test_emu: test_b
	@echo "[EMU] All emulator tests passed"

clean:
	@rm -rf out

# ── Output directory ───────────────────────────────────────────

out:
	@mkdir -p out

# ── Assembler tests ────────────────────────────────────────────

out/asm_z80_%: asm/z80/%.s | out
	@echo "  ASM z80   $*"
	@$(ASM_Z80) -o /dev/null $< > /dev/null 2>&1 && touch $@ || (echo "FAIL: $<"; exit 1)

out/asm_8080_%: asm/8080/%.s | out
	@echo "  ASM 8080  $*"
	@$(ASM_8080) -o /dev/null $< > /dev/null 2>&1 && touch $@ || (echo "FAIL: $<"; exit 1)

out/asm_8085_%: asm/8085/%.s | out
	@echo "  ASM 8085  $*"
	@$(ASM_8085) -o /dev/null $< > /dev/null 2>&1 && touch $@ || (echo "FAIL: $<"; exit 1)

out/asm_8086_%: asm/8086/%.s | out
	@echo "  ASM 8086  $*"
	@$(ASM_8086) -o /dev/null $< > /dev/null 2>&1 && touch $@ || (echo "FAIL: $<"; exit 1)

# ── B language tests (Z80/CP/M) ────────────────────────────────

.SECONDARY:

out/b_z80_%.com: b/%.b out/lib_z80.lib | out
	@echo "  B   z80   $*"
	@$(COM_Z80) -o out/b_z80_$*.s $< 2>/dev/null || (echo "FAIL COMPILE: $<"; exit 1)
	@$(ASM_Z80) -o out/b_z80_$*.obj out/b_z80_$*.s 2>/dev/null || (echo "FAIL ASM: $<"; exit 1)
	@$(LINK_BIN) -text 0x100 -o $@ out/b_z80_$*.obj out/lib_z80.lib 2>/dev/null || (echo "FAIL LINK: $<"; exit 1)

out/b_z80_%: out/b_z80_%.com
	@if [ -f b/$*.expect ]; then \
		echo "" | $(EMU_CPM) $< 2>&1 | tr -d '\r' | diff - b/$*.expect > /dev/null 2>&1 && touch $@ || ( \
			echo "FAIL OUTPUT: $*"; \
			echo "Expected:"; cat b/$*.expect; \
			echo "Got:"; echo "" | $(EMU_CPM) $< 2>&1; \
			exit 1 \
		); \
	else \
		echo "" | $(EMU_CPM) $< 2>&1 > /dev/null && touch $@ || (echo "FAIL RUN: $<"; exit 1); \
	fi

# ── B language tests (8080/CP/M) ────────────────────────────────

out/b_8080_%.com: b/%.b out/lib_8080.lib | out
	@echo "  B   8080  $*"
	@$(COM_8080) -o out/b_8080_$*.s $< 2>/dev/null || (echo "FAIL COMPILE: $<"; exit 1)
	@$(ASM_8080) -o out/b_8080_$*.obj out/b_8080_$*.s 2>/dev/null || (echo "FAIL ASM: $<"; exit 1)
	@$(LINK_BIN) -text 0x100 -o $@ out/b_8080_$*.obj out/lib_8080.lib 2>/dev/null || (echo "FAIL LINK: $<"; exit 1)

out/b_8080_%: out/b_8080_%.com
	@if [ -f b/$*.expect ]; then \
		echo "" | $(EMU_CPM) $< 2>&1 | tr -d '\r' | diff - b/$*.expect > /dev/null 2>&1 && touch $@ || ( \
			echo "FAIL OUTPUT: $*"; exit 1 \
		); \
	else \
		echo "" | $(EMU_CPM) $< 2>&1 > /dev/null && touch $@ || (echo "FAIL RUN: $<"; exit 1); \
	fi

# ── B language tests (8086/DOS) ──────────────────────────────────

out/b_8086_%.com: b/%.b out/lib_8086.lib | out
	@echo "  B   8086  $*"
	@$(COM_8086) -o out/b_8086_$*.s $< 2>/dev/null || (echo "FAIL COMPILE: $<"; exit 1)
	@$(ASM_8086) -o out/b_8086_$*.obj out/b_8086_$*.s 2>/dev/null || (echo "FAIL ASM: $<"; exit 1)
	@$(LINK_BIN) -text 0x100 -o $@ out/b_8086_$*.obj out/lib_8086.lib 2>/dev/null || (echo "FAIL LINK: $<"; exit 1)

out/b_8086_%: out/b_8086_%.com
	@if [ -f b/$*.expect ]; then \
		$(EMU_86) $< 2>&1 | tr -d '\r\n' | diff - <(tr -d '\n' < b/$*.expect) > /dev/null 2>&1 && touch $@ || ( \
			echo "FAIL OUTPUT: $*"; exit 1 \
		); \
	else \
		$(EMU_86) $< 2>&1 > /dev/null && touch $@ || (echo "FAIL RUN: $<"; exit 1); \
	fi

# ── Helper: build stub library with putchar + printf for tests ──

out/test_stub.b: | out
	@echo 'extrn putchar;' > $@
	@echo '_start() {' >> $@
	@echo '    main();' >> $@
	@echo '    putchar(10);' >> $@
	@echo '}' >> $@

out/lib_z80.lib: out/test_stub.b | out
	@$(COM_Z80) -o out/test_stub_z80.s out/test_stub.b 2>/dev/null
	@$(ASM_Z80) -o out/test_stub_z80.obj out/test_stub_z80.s 2>/dev/null
	@cp $(LIBS)/z80-cpm-b.lib out/lib_z80.lib 2>/dev/null || true

out/lib_8080.lib: out/test_stub.b | out
	@$(COM_8080) -o out/test_stub_8080.s out/test_stub.b 2>/dev/null
	@$(ASM_8080) -o out/test_stub_8080.obj out/test_stub_8080.s 2>/dev/null
	@cp $(LIBS)/8080-cpm-b.lib out/lib_8080.lib 2>/dev/null || true

out/lib_8086.lib: out/test_stub.b | out
	@$(COM_8086) -o out/test_stub_8086.s out/test_stub.b 2>/dev/null
	@$(ASM_8086) -o out/test_stub_8086.obj out/test_stub_8086.s 2>/dev/null
	@cp $(LIBS)/8086-msdos-b.lib out/lib_8086.lib 2>/dev/null || true
