
POSIX_PATH = ../bin/
POSIX_OUT = $(patsubst %,$(POSIX_PATH)%,$(OUT_BASE))
LINUX_PATH = ../bin/linux/
LINUX_OUT = $(patsubst %,$(LINUX_PATH)%,$(OUT_BASE))
MACOS_PATH = ../bin/macos/
MACOS_OUT = $(patsubst %,$(MACOS_PATH)%,$(OUT_BASE))
WIN_PATH = ../bin/win/
WIN_OUT = $(patsubst %,$(WIN_PATH)%.exe,$(OUT_BASE))
WIN32_PATH = ../bin/win32/
WIN32_OUT = $(patsubst %,$(WIN32_PATH)%.exe,$(OUT_BASE))
DOS_PATH = ../bin/dos/
DOS_OUT = $(patsubst %,$(DOS_PATH)%.exe,$(OUT_BASE))
DJGPP_PATH = /usr/local/djgpp


all test test_emu posix: $(POSIX_OUT)
	@true

samples_zip:
	@true

dos: $(filter-out $(DOS_PATH)retrolang-%.exe,$(filter-out $(DOS_PATH)msxdosemu.exe,$(DOS_OUT)))
	@echo $^
	@true

linux: $(LINUX_OUT)
	@true

macos: $(MACOS_OUT)
	@true

win: $(filter-out $(WIN_PATH)msxdosemu.exe,$(WIN_OUT))
	@true

win32: $(filter-out $(WIN32_PATH)msxdosemu.exe,$(WIN32_OUT))
	@true

clean:
	@rm -f $(POSIX_OUT) $(WIN_OUT) $(LINUX_OUT) $(MACOS_OUT) $(CLR)

define add_to_path
ifeq ($(findstring $(1),$(PATH)),)
    PATH := $(1):$(PATH)
endif
endef

# $(eval $(call add_to_path,$(DJGPP_PATH)/bin))


$(POSIX_OUT): $(SRC_REQS) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cc -g -o $@ $(SRC)

$(DOS_OUT):  $(SRC_REQS) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cp ../license $(@D)/license.txt
	@cp -R ../samples $(@D)/samples
	@DJDIR=$(DJGPP_PATH)/i586-pc-msdosdjgpp GCC_EXE_PREFIX=$(DJGPP_PATH)/lib/gcc $(DJGPP_PATH)/bin/i586-pc-msdosdjgpp-gcc -DDOS_HOST=1 -DEXE_EXT=1 -o $@ $(patsubst %.exe.c,%.c,$(SRC))

$(LINUX_OUT):  $(SRC_REQS) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cp ../license $(@D)/license
	@cp -R ../samples $(@D)/samples
	@x86_64-unknown-linux-gnu-gcc -m64 -static -o $@ $(SRC)

$(MACOS_OUT):  $(SRC_REQS) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cp ../license $(@D)/license.txt
	@cp -R ../samples $(@D)/samples
	@clang -arch x86_64 -arch arm64 -fno-common -o $@ $(SRC)

$(WIN_OUT):  $(patsubst %.exe.c,%.c,$(SRC_REQS)) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cp ../license $(@D)/license.txt
	@cp -R ../samples $(@D)/samples
	@x86_64-w64-mingw32-gcc -DWINDOWS_HOST=1 -DEXE_EXT=1 -o $@ $(patsubst %.exe.c,%.c,$(SRC))

$(WIN32_OUT):  $(patsubst %.exe.c,%.c,$(SRC_REQS)) Makefile
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cp ../license $(@D)/license.txt
	@cp -R ../samples $(@D)/samples
	@i686-w64-mingw32-gcc -DWINDOWS_HOST=1 -DEXE_EXT=1 -o $@ $(patsubst %.exe.c,%.c,$(SRC))