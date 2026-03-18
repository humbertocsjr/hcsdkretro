OUT_BASE = retrolang-8086 retrolang-z80
SRC_REQS = $(wildcard *.h *.c cpu/*.c ../include/*.h)
SRC = $(wildcard *.c) cpu/$(patsubst $(@D)/retrolang-%,%.c,$@)
CLR = *.o cpu/*.o
include ../common.mk