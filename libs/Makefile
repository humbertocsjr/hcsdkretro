OUT = start-dos-8086.obj start-cpm-z80.obj $(patsubst dos/%.prj,%.lib,$(wildcard dos/*.prj)) $(patsubst cpm/%.prj,%.lib,$(wildcard cpm/*.prj))

all posix linux macos win win32 samples_zip test test_emu: $(OUT)
	@true


clean: 
	@rm $(OUT) *.dump *.obj *.sym *.bin *.lib *.sym 2>/dev/null || true
	@rm $(patsubst %,%/z80/*.obj,common cpm) 2> /dev/null || true
	@rm $(patsubst %,%/8086/*.obj,common dos) 2> /dev/null || true

%.lib: dos/%.prj  $(wildcard common/*.rl common/8086/*.s common/8086/*.rl dos/8086/*.s dos/8086/*.rl dos/z80/*.s dos/z80/*.rl)
	@echo [BUILD] $@
	@../bin/hcbuild $< make release


%.lib: cpm/%.prj  $(wildcard common/*.rl common/z80/*.s common/z80/*.rl cpm/z80/*.s cpm/z80/*.rl)
	@echo [BUILD] $@
	@../bin/hcbuild $< make release

start-dos-8086.obj: dos/start-8086.s
	@echo [ASM] $@
	@../bin/hcasm-8086 -o $@ $<

start-cpm-z80.obj: cpm/start-z80.s
	@echo [ASM] $@
	@../bin/hcasm-z80 -o $@ $<


FORCE: