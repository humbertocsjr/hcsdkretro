M = @make --no-print-directory --silent $@ -C
INSTDIR = /usr/local/bin

all clean test posix linux macos win:
	$M hcasm
	$M hclink
	$M hclib
	$M hcbuild
	$M tests

install: all
	@install -d $(INSTDIR)
	@install $(filter-out %.dSYM,$(wildcard bin/hcasm-* bin/hclink-* bin/hclib bin/hcbuild)) $(INSTDIR)