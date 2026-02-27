OUT = $(patsubst %-lib.prj,%.lib,$(wildcard *-lib.prj)) $(patsubst %-bin.prj,%.bin,$(wildcard *-bin.prj)) $(patsubst %-com.prj,%.com,$(wildcard *-com.prj)) $(patsubst %-rex.prj,%.rex,$(wildcard *-rex.prj))

all:
	@true

test: $(OUT)
	@true

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