OUT = $(patsubst %-lib.prj,%.lib,$(wildcard *-lib.prj)) $(patsubst %-bin.prj,%.bin,$(wildcard *-bin.prj)) $(patsubst %-com.prj,%.com,$(wildcard *-com.prj)) $(patsubst %-rex.prj,%.rex,$(wildcard *-rex.prj))

all posix linux macos win win32 dos samples_zip:
	@true

test: $(OUT)
	@true

test_emu: test
	@z80dasm -a -l -g 0x100 -c -o z80-test-0.com.dump --tab-width=4 -t z80-test-0.com
	@../bin/msxdosemu -diska ./ -step z80-test-1.com test.txt

clean: 
	@rm $(OUT) *.dump *.obj *.sym *.bin *.lib *.sym 2>/dev/null || true

%.lib: %-lib.prj FORCE
	@echo [BUILD] $@
	@../bin/hcbuild $< make release

%.bin: %-bin.prj FORCE
	@echo [BUILD] $@
	@../bin/hcbuild $< make release

%.com: %-com.prj FORCE
	@echo [BUILD] $@
	@../bin/hcbuild $< make release

%.rex: %-rex.prj FORCE
	@echo [BUILD] $@
	@../bin/hcbuild $< make release


FORCE: