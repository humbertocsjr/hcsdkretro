# Build all B library variant samples

PRJS = z80cpm 8080cpm 8085cpm 86msdos 86msdos-exe

.PHONY: all clean

all: $(PRJS)

%:
	@echo [BUILD] $@
	@../../bin/hcbuild --sdk-path ../../bin --lib-path ../../libs $@.prj make release

clean:
	@rm -f $(addsuffix .COM,$(PRJS)) $(addsuffix .EXE,$(PRJS)) *.obj *.dump *.sym
