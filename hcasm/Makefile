OUT_BASE = hcasm-8080 hcasm-8085  hcasm-z80 hcasm-8086
SRC_REQS = $(wildcard *.h *.c cpu/*.c ../include/*.h)
SRC = $(wildcard *.c) cpu/$(patsubst $(@D)/hcasm-%,%.c,$@)
CLR = *.o cpu/*.o
include ../common.mk