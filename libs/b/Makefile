# B Language Runtime Library Makefile
# Builds .lib files for all target CPUs inside libs/b/
# Each .lib contains arch-specific runtime + generic B modules

# --== Tool paths (relative to libs/b/) ==--
ASM  = ../../bin
BCOM = ../../bin

# --== CPU targets ==--
Z80_ARCH   = z80
CPU_8080   = 8080
CPU_8085   = 8085
CPU_8086   = 8086


# --== Libraries to build (stored in libs/ root with standard names) ==--
BLIBS = \
	../z80-cpm-b.lib \
	../z80-msxdos2-b.lib \
	../8080-cpm-b.lib \
	../8085-cpm-b.lib \
	../8086-msdos-b.lib \
	../8086-msdos-exe-b.lib

# --== Generic B modules (compiled per CPU) ==--
B_COMMON = abs atoi gets itoa memcpy memset printf strcat strcmp strcpy strlen

# --== Arch-specific assembly modules (base path) ==--
ASM_COMMON = putchar getchar puts exit

# Z80-specific .s modules (CP/M file I/O + runtime math)
Z80_OBJS = \
	$(Z80_ARCH)/bdos.obj \
	$(Z80_ARCH)/cpm_start.obj \
	$(Z80_ARCH)/mul.obj \
	$(Z80_ARCH)/div8.obj \
	$(Z80_ARCH)/div16.obj \
	$(Z80_ARCH)/cmp8.obj \
	$(Z80_ARCH)/cmp16.obj \
	$(Z80_ARCH)/shift.obj \
	$(Z80_ARCH)/cbw.obj \
	$(Z80_ARCH)/cpm_data.obj \
	$(Z80_ARCH)/cpm_byte.obj \
	$(Z80_ARCH)/cpm_str.obj \
	$(Z80_ARCH)/cpm_slot.obj \
	$(Z80_ARCH)/cpm_name.obj \
	$(Z80_ARCH)/fopen.obj \
	$(Z80_ARCH)/fclose.obj \
	$(Z80_ARCH)/fread.obj \
	$(Z80_ARCH)/fwrite.obj \
	$(Z80_ARCH)/fdelete.obj \
	$(Z80_ARCH)/frename.obj \
	$(Z80_ARCH)/fseek.obj \
	$(Z80_ARCH)/ftell.obj \
	$(Z80_ARCH)/feof.obj

# --== All object files per CPU ==--
Z80_OBJECTS = \
	$(foreach m,$(B_COMMON),$(Z80_ARCH)/$(m).obj) \
	$(foreach m,$(ASM_COMMON),$(Z80_ARCH)/$(m).obj) \
	$(Z80_OBJS)

# MSX-DOS 2 Z80 library objects (reuses B_COMMON, ASM_COMMON, and math from Z80)
Z80_MSXDOS2_OBJECTS = \
	$(foreach m,$(B_COMMON),$(Z80_ARCH)/$(m).obj) \
	$(foreach m,$(ASM_COMMON),$(Z80_ARCH)/$(m).obj) \
	$(Z80_ARCH)/bdos.obj \
	$(Z80_ARCH)/mul.obj \
	$(Z80_ARCH)/div8.obj \
	$(Z80_ARCH)/div16.obj \
	$(Z80_ARCH)/cmp8.obj \
	$(Z80_ARCH)/cmp16.obj \
	$(Z80_ARCH)/shift.obj \
	$(Z80_ARCH)/cbw.obj \
	$(Z80_ARCH)/msxdos2_start.obj \
	$(Z80_ARCH)/msxdos2_file.obj

CPU_8080_OBJS = \
	$(CPU_8080)/mul.obj \
	$(CPU_8080)/div16.obj \
	$(CPU_8080)/div8.obj \
	$(CPU_8080)/cmp8.obj \
	$(CPU_8080)/cmp16.obj \
	$(CPU_8080)/shift.obj \
	$(CPU_8080)/cbw.obj

CPU_8080_OBJECTS = \
	$(foreach m,$(B_COMMON),$(CPU_8080)/$(m).obj) \
	$(foreach m,$(ASM_COMMON),$(CPU_8080)/$(m).obj) \
	$(CPU_8080)/bdos.obj \
	$(CPU_8080)/cpm_start.obj \
	$(CPU_8080_OBJS)

CPU_8085_OBJS = \
	$(CPU_8085)/mul.obj \
	$(CPU_8085)/div16.obj \
	$(CPU_8085)/div8.obj \
	$(CPU_8085)/cmp8.obj \
	$(CPU_8085)/cmp16.obj \
	$(CPU_8085)/shift.obj \
	$(CPU_8085)/cbw.obj

CPU_8085_OBJECTS = \
	$(foreach m,$(B_COMMON),$(CPU_8085)/$(m).obj) \
	$(foreach m,$(ASM_COMMON),$(CPU_8085)/$(m).obj) \
	$(CPU_8085)/bdos.obj \
	$(CPU_8085)/cpm_start.obj \
	$(CPU_8085_OBJS)

CPU_8086_OBJECTS = \
	$(foreach m,$(B_COMMON),$(CPU_8086)/$(m).obj) \
	$(foreach m,$(ASM_COMMON),$(CPU_8086)/$(m).obj) \
	$(CPU_8086)/dos_file.obj \
	$(CPU_8086)/msdos_start.obj

CPU_8086_EXE_OBJECTS = \
	$(foreach m,$(B_COMMON),$(CPU_8086EXE)/$(m).obj) \
	$(CPU_8086EXE)/io.obj \
	$(CPU_8086EXE)/mul32.obj \
	$(CPU_8086EXE)/div32.obj \
	$(CPU_8086EXE)/sh32.obj \
	$(CPU_8086)/msdos_start_mz.obj

# --== Generic B source files ==--
B_SRCS = $(foreach m,$(B_COMMON),$(m).b)

# --== ASM source files exist as .s files ==--
Z80_ASM_SRCS = $(foreach m,$(ASM_COMMON),$(Z80_ARCH)/$(m).s) $(Z80_ARCH)/bdos.s $(Z80_ARCH)/start.s

# --== Targets ==--
.PHONY: all clean posix linux macos win win32 dos

# All host targets build the same B libraries
all posix linux macos win win32 dos: $(BLIBS)
	@true

test test_emu:
	@true

# --== Pattern: compile .b -> .s -> .obj for a given CPU ==--
# Each .b file must go through hcbcomp-<cpu>, then hcasm-<cpu>

define b_compile_template
# $$1 = cpu arch (e.g., z80), $$2 = module name (e.g., strlen)
$(1)/$(2).__s: $(2).b Makefile
	@mkdir -p $(1)
	@echo [B]   $(1)/$(2).__s
	@$(BCOM)/hcbcomp-$(patsubst 8086exe,8086exe,$(patsubst 8085,8080,$(1))) -o $$@ $$< 2>/dev/null

$(1)/$(2).obj: $(1)/$(2).__s Makefile
	@echo [ASM] $(1)/$(2).obj
	@$(ASM)/hcasm-$(patsubst 8086exe,8086,$(patsubst 8085,8080,$(1))) -o $$@ $$< 2>/dev/null
endef

# Generate compile rules for each B module for each CPU
$(foreach cpu,z80 8080 8085 8086 8086exe,\
  $(foreach mod,$(B_COMMON),\
    $(eval $(call b_compile_template,$(cpu),$(mod)))))

# --== Pattern: assemble .s -> .obj for arch-specific modules ==--
# Z80-specific B modules (compile .b -> .__s -> .obj)
Z80_BMODS = cpm_data cpm_byte cpm_str cpm_slot cpm_name fopen fclose fread fwrite fdelete frename fseek ftell feof

$(Z80_ARCH)/%.__s: $(Z80_ARCH)/%.b Makefile
	@echo [B]   $@
	@$(BCOM)/hcbcomp-z80 -o $@ $< 2>/dev/null

$(Z80_ARCH)/%.obj: $(Z80_ARCH)/%.__s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-z80 -o $@ $< 2>/dev/null

# Z80 assembly modules
$(Z80_ARCH)/bdos.obj: $(Z80_ARCH)/bdos.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-z80 -o $@ $< 2>/dev/null

$(Z80_ARCH)/start.obj: $(Z80_ARCH)/start.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-z80 -o $@ $< 2>/dev/null

$(Z80_ARCH)/%.obj: $(Z80_ARCH)/%.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-z80 -o $@ $< 2>/dev/null

# 8080 assembly modules
$(CPU_8080)/%.obj: $(CPU_8080)/%.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-8080 -o $@ $< 2>/dev/null

# 8085 assembly modules
$(CPU_8085)/%.obj: $(CPU_8085)/%.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-8085 -o $@ $< 2>/dev/null

# 8086 assembly modules
$(CPU_8086)/%.obj: $(CPU_8086)/%.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-8086 -o $@ $< 2>/dev/null

$(CPU_8086)/msdos_start_mz.obj: $(CPU_8086)/msdos_start_mz.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-8086 -o $@ $< 2>/dev/null

# 8086exe assembly modules (MZ EXE specific: IO + math)
CPU_8086EXE = 8086exe
$(CPU_8086EXE)/%.obj: $(CPU_8086EXE)/%.s Makefile
	@echo [ASM] $@
	@$(ASM)/hcasm-8086 -o $@ $< 2>/dev/null

# --== Library packaging (output to libs/ root) ==--
../z80-cpm-b.lib: $(Z80_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(Z80_OBJECTS) 2>/dev/null

../z80-msxdos2-b.lib: $(Z80_MSXDOS2_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(Z80_MSXDOS2_OBJECTS) 2>/dev/null

../8080-cpm-b.lib: $(CPU_8080_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(CPU_8080_OBJECTS) 2>/dev/null

../8085-cpm-b.lib: $(CPU_8085_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(CPU_8085_OBJECTS) 2>/dev/null

../8086-msdos-b.lib: $(CPU_8086_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(CPU_8086_OBJECTS) 2>/dev/null

../8086-msdos-exe-b.lib: $(CPU_8086_EXE_OBJECTS)
	@echo [LIB] $@
	@$(ASM)/hclib $@ $(CPU_8086_EXE_OBJECTS) 2>/dev/null

# --== Clean: remove generated files only, keep hand-written .s sources ==--
clean:
	@rm -f *.obj $(Z80_ARCH)/*.obj $(CPU_8080)/*.obj $(CPU_8085)/*.obj $(CPU_8086)/*.obj $(CPU_8086EXE)/*.obj
	@rm -f *.__s $(Z80_ARCH)/*.__s $(CPU_8086)/*.__s $(CPU_8086EXE)/*.__s
	@rm -f $(BLIBS)
	@rm -f ../z80-cpm-start.obj
	@rm -f $(CPU_8086)/msdos_start_mz.obj
