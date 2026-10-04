# Builds the Warp3D test programs with m68k-amigaos-gcc.
# Run from the repository root:  make -f tests/Makefile  (or ./build.sh tests)

CC      = m68k-amigaos-gcc
BUILD  ?= build/tests/amiga

CPU     = -m68020-60 -m68881
CFLAGS  = $(CPU) -O2 -Wall -Wno-pointer-sign -Werror=implicit-function-declaration \
          -Itests/include -IWarp3D.library
LDFLAGS = $(CPU)

TESTS   = $(basename $(notdir $(wildcard tests/t[0-9][0-9]_*.c)))
HEADERS = tests/common.h tests/include/warp3d_calls.h Warp3D.library/Warp3D.h

all: $(addprefix $(BUILD)/,$(TESTS))

$(BUILD)/common.o: tests/common.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: tests/%.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%: $(BUILD)/%.o $(BUILD)/common.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $@

.PRECIOUS: $(BUILD)/%.o

.PHONY: all
