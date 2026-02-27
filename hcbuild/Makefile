EXE_PATH = ../bin/
EXE = $(EXE_PATH)hcbuild

all test: $(EXE)
	@true

clean:
	@rm -f $(EXE) *.o 

$(EXE): $(wildcard *.h *.c  ../include/*.h)
	@echo [CC] $(@F)
	@mkdir -p $(EXE_PATH)
	@cc -g -o $@ $(wildcard *.c)