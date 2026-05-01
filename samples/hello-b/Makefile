# Build all B library variant samples

PRJS = z80cpm 8080cpm 8085cpm 86msdos

.PHONY: all clean

all: $(PRJS)

%:
	@echo [BUILD] $@
	@../../bin/hcbuild --sdk-path ../../bin $@.prj make release

clean:
	@rm -f $(addsuffix .com,$(PRJS)) *.obj *.dump *.sym
