OUT_BASE = hclink-bin hclink-rex
SRC_REQS = $(wildcard *.h *.c  ../include/*.h format/*.c)
SRC = $(wildcard *.c) $(patsubst $(@D)/hclink-%,format/%.c,$@)
CLR = *.o format/*.o
include ../common.mk