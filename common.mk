
POSIX_PATH = ../bin/
POSIX_OUT = $(patsubst %,$(POSIX_PATH)%,$(OUT_BASE))
LINUX_PATH = ../bin/linux/
LINUX_OUT = $(patsubst %,$(LINUX_PATH)%,$(OUT_BASE))
MACOS_PATH = ../bin/macos/
MACOS_OUT = $(patsubst %,$(MACOS_PATH)%,$(OUT_BASE))
WIN_PATH = ../bin/win/
WIN_OUT = $(patsubst %,$(WIN_PATH)%.exe,$(OUT_BASE))


all test posix: $(POSIX_OUT)
	@true

linux: $(LINUX_OUT)
	@true

macos: $(MACOS_OUT)
	@true

win: $(WIN_OUT)
	@true

clean:
	@rm -f $(POSIX_OUT) $(CLR)

$(POSIX_OUT): $(SRC_REQS)
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@cc -g -o $@ $(SRC)

$(LINUX_OUT):  $(SRC_REQS)
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@x86_64-unknown-linux-gnu-gcc -m64 -o $@ $(SRC)

$(MACOS_OUT):  $(SRC_REQS)
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@clang -arch x86_64 -arch arm64 -o $@ $(SRC)

$(WIN_OUT):  $(patsubst %.exe.c,%.c,$(SRC_REQS))
	@echo [CC] $(@F)
	@mkdir -p $(@D)
	@x86_64-w64-mingw32-gcc -o $@ $(patsubst %.exe.c,%.c,$(SRC))