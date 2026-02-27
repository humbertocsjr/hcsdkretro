EXE_PATH = ../bin/
EXE = $(EXE_PATH)hclib

all test: $(EXE)
	@true

clean:
	@rm -f $(EXE) *.o

$(EXE): $(wildcard *.h *.c  ../include/*.h)
	@echo [CC] $(@F)
	@mkdir -p $(EXE_PATH)
	@cc -g -o $@ $(wildcard *.c)