EXE_PATH = ../bin/
EXE = $(EXE_PATH)hcasm-8080 $(EXE_PATH)hcasm-8085  $(EXE_PATH)hcasm-z80 $(EXE_PATH)hcasm-8086

all test: $(EXE)
	@true

clean:
	@rm -f $(EXE) *.o cpu/*.o

$(EXE): $(wildcard *.h *.c cpu/*.c ../include/*.h)
	@echo [CC] $(@F)
	@mkdir -p $(EXE_PATH)
	@cc -g -o $@ $(wildcard *.c) cpu/$(patsubst $(EXE_PATH)hcasm-%,%.c,$@)