EXE_PATH = ../bin/
EXE = $(EXE_PATH)hclink-bin $(EXE_PATH)hclink-rex

all test: $(EXE)
	@true

clean:
	@rm -f $(EXE) *.o cpu/*.o

$(EXE): $(wildcard *.h *.c  ../include/*.h) $(patsubst $(EXE_PATH)hclink-%,format/%.c,$@)
	@echo [CC] $(@F)
	@mkdir -p $(EXE_PATH)
	@cc -g -o $@ $(wildcard *.c) $(patsubst $(EXE_PATH)hclink-%,format/%.c,$@)