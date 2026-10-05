# Builds the Warp3D (tNN_*) and agl.library (aNN_*) test programs, and the
# minigl.library checks (mNN_*) when MiniGL's SDK headers are there, with
# m68k-amigaos-gcc. Run from the repository root:
#   make -f tests/Makefile  (or ./build.sh tests)

CC      = m68k-amigaos-gcc
PYTHON ?= python3
BUILD  ?= build/tests/amiga

CPU     = -m68020-60 -m68881
CFLAGS  = $(CPU) -O2 -Wall -Wno-pointer-sign -Werror=implicit-function-declaration \
          -Itests -Itests/include -IWarp3D.library -Igl -I$(BUILD)
LDFLAGS = $(CPU)

W3D_TESTS   = $(basename $(notdir $(wildcard tests/t[0-9][0-9]_*.c)))
AGL_TESTS   = $(basename $(notdir $(wildcard tests/a[0-9][0-9]_*.c)))
MGL_TESTS   = $(basename $(notdir $(wildcard tests/m[0-9][0-9]_*.c)))
MINIGL_SDK ?= /sdk
MGL_CFLAGS  = $(CPU) -O2 -Wall -Wno-pointer-sign -Werror=implicit-function-declaration \
              -Iminigl.library/sdk-shim -I$(MINIGL_SDK)
MGL_BUILT   = $(if $(wildcard $(MINIGL_SDK)/libraries/minigl_dispatch.h),$(MGL_TESTS))
W3D_HEADERS = tests/common.h tests/window.h tests/include/warp3d_calls.h Warp3D.library/Warp3D.h
AGL_HEADERS = tests/agl_common.h tests/window.h $(BUILD)/agl_calls.h gl/gl.h gl/gldefines.h

all: $(addprefix $(BUILD)/,$(W3D_TESTS) $(AGL_TESTS) $(MGL_BUILT))

$(BUILD)/agl_calls.h: agl.library/agl_lib.fd tests/aglcalls.py | $(BUILD)
	$(PYTHON) tests/aglcalls.py $< $@

$(BUILD)/window.o: tests/window.c tests/window.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/common.o: tests/common.c $(W3D_HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/agl_common.o: tests/agl_common.c $(AGL_HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/t%.o: tests/t%.c $(W3D_HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/a%.o: tests/a%.c $(AGL_HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/t%: $(BUILD)/t%.o $(BUILD)/common.o $(BUILD)/window.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/a%: $(BUILD)/a%.o $(BUILD)/agl_common.o $(BUILD)/window.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/m%: tests/m%.c minigl.library/sdk-shim/mgl/context.h | $(BUILD)
	$(CC) $(MGL_CFLAGS) -o $@ $<

$(BUILD):
	mkdir -p $@

.PRECIOUS: $(BUILD)/t%.o $(BUILD)/a%.o

.PHONY: all
