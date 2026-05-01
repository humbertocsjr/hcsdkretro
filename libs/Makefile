# Libraries Makefile
# Builds B language runtime libs (in libs/ root)

BLIBS = z80-cpm-b.lib 8080-cpm-b.lib 8085-cpm-b.lib 8086-msdos-b.lib

.PHONY: all clean posix linux macos win win32 dos test test_emu samples_zip

all posix linux macos win win32 dos test test_emu:
	@make --no-print-directory $@ -C b

samples_zip:
	@true

clean:
	@make --no-print-directory clean -C b
	@rm -f $(BLIBS)
