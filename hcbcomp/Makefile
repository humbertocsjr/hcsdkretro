OUT_BASE = hcbcomp-z80 hcbcomp-8086 hcbcomp-8080 hcbcomp-8085
SRC_REQS = $(wildcard *.h *.c gen/*.c ../include/*.h)
SRC = $(wildcard *.c) gen/$(patsubst $(@D)/hcbcomp-%,%.c,$@)
CLR = *.o gen/*.o
include ../common.mk
