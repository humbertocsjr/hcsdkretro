OUT = $(patsubst dos/%.prj,%.lib,$(wildcard dos/*.prj)) $(patsubst cpm/%.prj,%.lib,$(wildcard cpm/*.prj))

all posix linux macos win win32 samples_zip: $(OUT)
	@true

test: 
	@true

clean: 
	@rm $(OUT) *.dump *.obj *.sym *.bin *.lib *.sym 2>/dev/null || true
	@rm $(patsubst %,%/z80/*.obj,common cpm) 2> /dev/null || true
	@rm $(patsubst %,%/8086/*.obj,common dos) 2> /dev/null || true

%.lib: dos/%.prj  $(wildcard dos/8086/*.s dos/8086/*.rl dos/z80/*.s dos/z80/*.rl)
	@echo [BUILD] $@
	@../bin/hcbuild $< make release


%.lib: cpm/%.prj  $(wildcard cpm/z80/*.s cpm/z80/*.rl)
	@echo [BUILD] $@
	@../bin/hcbuild $< make release


FORCE: