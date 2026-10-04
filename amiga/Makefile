# Builds Warp3D.library and agl.library with m68k-amigaos-gcc (bebbo).
# Run from the repository root:  make -f amiga/Makefile
# or through Docker:             build-amiga.sh

CC      = m68k-amigaos-gcc
PYTHON ?= python3
BUILD  ?= build/amiga

# The FPU is required: agl.library takes its float arguments in fp0-fp7.
CPU     = -m68020-60 -m68881
CFLAGS  = $(CPU) -O2 -fomit-frame-pointer -fcommon -fno-toplevel-reorder \
          -Wall -Wno-unused -Wno-pointer-sign -Wno-unknown-pragmas \
          -Wno-int-conversion -Werror=implicit-function-declaration \
          -Iamiga/include -include amiga/compat.h
LDFLAGS = $(CPU) -nostartfiles -nostdlib
LIBS    = -lgcc

VERSION  = 4
REVISION = 2
DATE     = $(shell date +%d.%m.%Y)

WARP3D_SRC = $(wildcard Warp3D.library/*.c)
AGL_SRC    = $(wildcard agl.library/*.c)
GL_SRC     = gl/gl.c

WARP3D_DIR = $(BUILD)/Warp3D
AGL_DIR    = $(BUILD)/agl

WARP3D_OBJ = $(WARP3D_DIR)/libheader.o $(patsubst %.c,$(WARP3D_DIR)/%.o,$(notdir $(WARP3D_SRC) $(GL_SRC))) $(WARP3D_DIR)/libc_shim.o
AGL_OBJ    = $(AGL_DIR)/libheader.o $(patsubst %.c,$(AGL_DIR)/%.o,$(notdir $(AGL_SRC) $(GL_SRC))) $(AGL_DIR)/libc_shim.o

GL_HEADERS = gl/gl.h gl/gldefines.h gl/gldeclarations.auto.h

all: $(BUILD)/Warp3D.library $(BUILD)/agl.library

$(BUILD)/Warp3D.library: $(WARP3D_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/agl.library: $(AGL_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# --- Warp3D.library ---------------------------------------------------------

$(WARP3D_DIR)/functable.h: Warp3D.library/Warp3D.fd amiga/fd2functable.py | $(WARP3D_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

$(WARP3D_DIR)/libheader.o: amiga/libheader.c $(WARP3D_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(WARP3D_DIR) -DLIB_NAME='"Warp3D.library"' \
		-DLIB_VERSION=$(VERSION) -DLIB_REVISION=$(REVISION) \
		-DLIB_IDSTRING='"Warp3D.library $(VERSION).$(REVISION) ($(DATE)) QuarkTex"' \
		-DLIB_INIT=INIT_0_Warp3D -DLIB_EXIT=EXIT_0_Warp3D -c -o $@ $<

$(WARP3D_DIR)/%.o: Warp3D.library/%.c Warp3D.library/w3d.h Warp3D.library/Warp3D.h $(GL_HEADERS) | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR)/gl.o: gl/gl.c gl/gldefinitions.auto.c gl/glDLLfunc.auto.c gl/glstatichandles.auto.c $(GL_HEADERS) | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR)/libc_shim.o: amiga/libc_shim.c | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# --- agl.library (StormMESA) ------------------------------------------------

$(AGL_DIR)/functable.h: agl.library/agl_lib.fd amiga/fd2functable.py | $(AGL_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

$(AGL_DIR)/libheader.o: amiga/libheader.c $(AGL_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(AGL_DIR) -DLIB_NAME='"agl.library"' \
		-DLIB_VERSION=$(VERSION) -DLIB_REVISION=$(REVISION) \
		-DLIB_IDSTRING='"agl.library $(VERSION).$(REVISION) ($(DATE)) QuarkTex"' \
		-DLIB_INIT=INIT_0_StormMESA -DLIB_EXIT=EXIT_0_StormMESA -c -o $@ $<

$(AGL_DIR)/%.o: agl.library/%.c $(GL_HEADERS) | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(AGL_DIR)/gl.o: gl/gl.c gl/gldefinitions.auto.c gl/glDLLfunc.auto.c gl/glstatichandles.auto.c $(GL_HEADERS) | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(AGL_DIR)/libc_shim.o: amiga/libc_shim.c | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR) $(AGL_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
