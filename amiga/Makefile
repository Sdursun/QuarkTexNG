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
          -Iamiga/include -include amiga/compat.h -DQT_PRODUCT='"$(PRODUCT)"'
LDFLAGS = $(CPU) -nostartfiles -nostdlib
LIBS    = -lgcc

# The product and its version, in the libraries' id strings and in what
# Warp3D, agl and minigl report as their driver or renderer (QT_PRODUCT).
PRODUCT  = QuarkTex NG 1.1

VERSION  = 4
REVISION = 2
DATE     = $(shell date +%d.%m.%Y)

WARP3D_SRC = $(wildcard Warp3D.library/*.c)
AGL_SRC    = $(wildcard agl.library/*.c)
GL_SRC     = gl/gl.c

# minigl.library needs MiniGL's 29 or 30 SDK headers (not part of QuarkTex); build.sh
# mounts them at /sdk and runs minigl.library/mglgen.py first. A program built
# on an older SDK is not supported (minigl.library/lib.c).
MINIGL_SDK ?= /sdk
MINIGL_SRC  = $(wildcard minigl.library/*.c)
# MINIGL_BACKEND_FLAG_CLASSIC, and QuarkTex's own bit (docs/phase7-minigl.md).
MINIGL_FLAGS = 0x102

# ogles2.library needs the PiStorm/Emu68 ogles2 SDK's root (not part of
# QuarkTex either); build.sh mounts it at /sdk-ogles2 and runs
# ogles2.library/oglesgen.py first (docs/phase9-ogles2.md).
OGLES2_SDK ?= /sdk-ogles2
OGLES2_SRC  = $(wildcard ogles2.library/*.c)

# ogles3.library: ogles2.library's sources (but its lib.c) built against the
# ogles3 SDK, mounted at /sdk-ogles3, with ogles3.library/*.c on top
# (docs/phase10-ogles3.md).
OGLES3_SDK ?= /sdk-ogles3
OGLES3_SHARED = $(filter-out ogles2.library/lib.c,$(OGLES2_SRC))
OGLES3_SRC  = $(wildcard ogles3.library/*.c)

WARP3D_DIR = $(BUILD)/Warp3D
MINIGL_DIR = $(BUILD)/minigl
AGL_DIR    = $(BUILD)/agl
OGLES2_DIR = $(BUILD)/ogles2
OGLES3_DIR = $(BUILD)/ogles3

WARP3D_OBJ = $(WARP3D_DIR)/libheader.o $(patsubst %.c,$(WARP3D_DIR)/%.o,$(notdir $(WARP3D_SRC) $(GL_SRC))) $(WARP3D_DIR)/libc_shim.o
AGL_OBJ    = $(AGL_DIR)/libheader.o $(patsubst %.c,$(AGL_DIR)/%.o,$(notdir $(AGL_SRC) $(GL_SRC))) $(AGL_DIR)/libc_shim.o
MINIGL_OBJ = $(MINIGL_DIR)/libheader.o $(patsubst %.c,$(MINIGL_DIR)/%.o,$(notdir $(MINIGL_SRC) $(GL_SRC))) \
             $(MINIGL_DIR)/dispatch.auto.o $(MINIGL_DIR)/mglenum.auto.o $(MINIGL_DIR)/libc_shim.o
OGLES2_OBJ = $(OGLES2_DIR)/libheader.o $(patsubst %.c,$(OGLES2_DIR)/%.o,$(notdir $(OGLES2_SRC) $(GL_SRC))) \
             $(OGLES2_DIR)/dispatch.auto.o $(OGLES2_DIR)/libc_shim.o
OGLES3_OBJ = $(OGLES3_DIR)/libheader.o $(patsubst %.c,$(OGLES3_DIR)/%.o,$(notdir $(OGLES3_SHARED) $(GL_SRC))) \
             $(patsubst %.c,$(OGLES3_DIR)/%3.o,$(notdir $(OGLES3_SRC))) \
             $(OGLES3_DIR)/dispatch.auto.o $(OGLES3_DIR)/libc_shim.o

GL_HEADERS = gl/gl.h gl/gldefines.h gl/gldeclarations.auto.h gl/w3dcmd.h

all: $(BUILD)/Warp3D.library $(BUILD)/agl.library
ifneq ($(wildcard $(MINIGL_SDK)/libraries/minigl_dispatch.h),)
all: $(BUILD)/minigl.library
endif
ifneq ($(wildcard $(OGLES2_SDK)/include/libraries/ogles2_dispatch.h),)
all: $(BUILD)/ogles2.library
endif
ifneq ($(wildcard $(OGLES3_SDK)/include/libraries/ogles3_dispatch.h),)
all: $(BUILD)/ogles3.library
endif

# PRODUCT, the versions and the flags are set here: a change rebuilds all.
$(WARP3D_OBJ) $(AGL_OBJ) $(MINIGL_OBJ) $(OGLES2_OBJ) $(OGLES3_OBJ): amiga/Makefile

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
		-DLIB_IDSTRING='"Warp3D.library $(VERSION).$(REVISION) ($(DATE)) $(PRODUCT)"' \
		-DLIB_INIT=INIT_0_Warp3D -DLIB_EXIT=EXIT_0_Warp3D -c -o $@ $<

$(WARP3D_DIR)/%.o: Warp3D.library/%.c Warp3D.library/w3d.h Warp3D.library/Warp3D.h $(GL_HEADERS) | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR)/gl.o: gl/gl.c gl/glencode.auto.c $(GL_HEADERS) | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR)/libc_shim.o: amiga/libc_shim.c | $(WARP3D_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# --- agl.library (StormMESA) ------------------------------------------------

$(AGL_DIR)/functable.h: agl.library/agl_lib.fd amiga/fd2functable.py | $(AGL_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

$(AGL_DIR)/libheader.o: amiga/libheader.c $(AGL_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(AGL_DIR) -DLIB_NAME='"agl.library"' \
		-DLIB_VERSION=$(VERSION) -DLIB_REVISION=$(REVISION) \
		-DLIB_IDSTRING='"agl.library $(VERSION).$(REVISION) ($(DATE)) $(PRODUCT)"' \
		-DLIB_INIT=INIT_0_StormMESA -DLIB_EXIT=EXIT_0_StormMESA -c -o $@ $<

$(AGL_DIR)/%.o: agl.library/%.c $(GL_HEADERS) | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(AGL_DIR)/gl.o: gl/gl.c gl/glencode.auto.c $(GL_HEADERS) | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(AGL_DIR)/libc_shim.o: amiga/libc_shim.c | $(AGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# --- minigl.library (MiniGL's shared library interface) ---------------------
# The *.auto.* files come from minigl.library/mglgen.py (run by build.sh).

MINIGL_CFLAGS = $(CFLAGS) -Iminigl.library/sdk-shim -I$(MINIGL_SDK) -I$(MINIGL_DIR) -Iminigl.library -Igl -DMGL_QUARKTEX_BACKEND_FLAGS=$(MINIGL_FLAGS)
MINIGL_GENERATED = $(MINIGL_DIR)/qgl.auto.h $(MINIGL_DIR)/dispatch.auto.c $(MINIGL_DIR)/mglenum.auto.c

$(BUILD)/minigl.library: $(MINIGL_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(MINIGL_GENERATED):
	@echo "$@ is missing: run minigl.library/mglgen.py (./build.sh does)" >&2; exit 1

$(MINIGL_DIR)/functable.h: minigl.library/minigl.fd amiga/fd2functable.py | $(MINIGL_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

# The version of PiStorm3D's minigl.library it matches (30.0; the interface is
# the 29 SDK's, unchanged in 30).
MINIGL_VERSION  = 30
MINIGL_REVISION = 0

$(MINIGL_DIR)/libheader.o: amiga/libheader.c $(MINIGL_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(MINIGL_DIR) -DLIB_NAME='"minigl.library"' \
		-DLIB_VERSION=$(MINIGL_VERSION) -DLIB_REVISION=$(MINIGL_REVISION) \
		-DLIB_IDSTRING='"minigl.library $(MINIGL_VERSION).$(MINIGL_REVISION) ($(DATE)) $(PRODUCT)"' \
		-DLIB_INIT=INIT_0_MiniGL -DLIB_EXIT=EXIT_0_MiniGL -c -o $@ $<

$(MINIGL_DIR)/%.o: minigl.library/%.c minigl.library/mgl.h $(MINIGL_DIR)/qgl.auto.h | $(MINIGL_DIR)
	$(CC) $(MINIGL_CFLAGS) -c -o $@ $<

$(MINIGL_DIR)/%.auto.o: $(MINIGL_DIR)/%.auto.c $(MINIGL_DIR)/qgl.auto.h
	$(CC) $(MINIGL_CFLAGS) -c -o $@ $<

$(MINIGL_DIR)/gl.o: gl/gl.c gl/glencode.auto.c $(GL_HEADERS) | $(MINIGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MINIGL_DIR)/libc_shim.o: amiga/libc_shim.c | $(MINIGL_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# --- ogles2.library (the PiStorm/Emu68 ogles2.library's shared library
# interface, stage 1: docs/phase9-ogles2.md) ---------------------------------
# The *.auto.* files come from ogles2.library/oglesgen.py (run by build.sh).

OGLES2_CFLAGS = $(CFLAGS) -I$(OGLES2_SDK)/include -I$(OGLES2_DIR) -Iogles2.library -Igl \
		-DQT_LIB_VERSION=$(OGLES2_VERSION) -DQT_LIB_REVISION=$(OGLES2_REVISION)
OGLES2_GENERATED = $(OGLES2_DIR)/dispatch.auto.c

$(BUILD)/ogles2.library: $(OGLES2_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OGLES2_GENERATED):
	@echo "$@ is missing: run ogles2.library/oglesgen.py (./build.sh does)" >&2; exit 1

$(OGLES2_DIR)/functable.h: ogles2.library/ogles2.fd amiga/fd2functable.py | $(OGLES2_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

OGLES2_VERSION  = 1
OGLES2_REVISION = 3

$(OGLES2_DIR)/libheader.o: amiga/libheader.c $(OGLES2_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(OGLES2_DIR) -DLIB_NAME='"ogles2.library"' \
		-DLIB_VERSION=$(OGLES2_VERSION) -DLIB_REVISION=$(OGLES2_REVISION) \
		-DLIB_IDSTRING='"ogles2.library $(OGLES2_VERSION).$(OGLES2_REVISION) ($(DATE)) $(PRODUCT)"' \
		-DLIB_INIT=INIT_0_OGLES2 -DLIB_EXIT=EXIT_0_OGLES2 -c -o $@ $<

$(OGLES2_DIR)/%.o: ogles2.library/%.c ogles2.library/ogles2.h $(OGLES2_DIR)/dispatch.auto.c | $(OGLES2_DIR)
	$(CC) $(OGLES2_CFLAGS) -c -o $@ $<

$(OGLES2_DIR)/%.auto.o: $(OGLES2_DIR)/%.auto.c ogles2.library/ogles2.h
	$(CC) $(OGLES2_CFLAGS) -c -o $@ $<

$(OGLES2_DIR)/gl.o: gl/gl.c gl/glencode.auto.c $(GL_HEADERS) | $(OGLES2_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OGLES2_DIR)/libc_shim.o: amiga/libc_shim.c | $(OGLES2_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# --- ogles3.library (docs/phase10-ogles3.md) -----------------------------------
# ogles2.library's objects again, with QT_OGLES3 and the ogles3 SDK; this
# directory's own files get a 3 in their object names (lib.c is in both).

OGLES3_CFLAGS = $(CFLAGS) -DQT_OGLES3 -I$(OGLES3_SDK)/include -I$(OGLES3_DIR) -Iogles2.library -Iogles3.library -Igl \
		-DQT_LIB_VERSION=$(OGLES3_VERSION) -DQT_LIB_REVISION=$(OGLES3_REVISION)

$(BUILD)/ogles3.library: $(OGLES3_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OGLES3_DIR)/dispatch.auto.c:
	@echo "$@ is missing: run ogles2.library/oglesgen.py --ogles3 (./build.sh does)" >&2; exit 1

$(OGLES3_DIR)/functable.h: ogles3.library/ogles3.fd amiga/fd2functable.py | $(OGLES3_DIR)
	$(PYTHON) amiga/fd2functable.py $< $@

OGLES3_VERSION  = 1
OGLES3_REVISION = 0

$(OGLES3_DIR)/libheader.o: amiga/libheader.c $(OGLES3_DIR)/functable.h
	$(CC) $(CFLAGS) -I$(OGLES3_DIR) -DLIB_NAME='"ogles3.library"' \
		-DLIB_VERSION=$(OGLES3_VERSION) -DLIB_REVISION=$(OGLES3_REVISION) \
		-DLIB_IDSTRING='"ogles3.library $(OGLES3_VERSION).$(OGLES3_REVISION) ($(DATE)) $(PRODUCT)"' \
		-DLIB_INIT=INIT_0_OGLES3 -DLIB_EXIT=EXIT_0_OGLES3 -c -o $@ $<

$(OGLES3_DIR)/%.o: ogles2.library/%.c ogles2.library/ogles2.h $(OGLES3_DIR)/dispatch.auto.c | $(OGLES3_DIR)
	$(CC) $(OGLES3_CFLAGS) -c -o $@ $<

$(OGLES3_DIR)/%3.o: ogles3.library/%.c ogles2.library/ogles2.h $(OGLES3_DIR)/dispatch.auto.c | $(OGLES3_DIR)
	$(CC) $(OGLES3_CFLAGS) -c -o $@ $<

$(OGLES3_DIR)/%.auto.o: $(OGLES3_DIR)/%.auto.c ogles2.library/ogles2.h
	$(CC) $(OGLES3_CFLAGS) -c -o $@ $<

$(OGLES3_DIR)/gl.o: gl/gl.c gl/glencode.auto.c $(GL_HEADERS) | $(OGLES3_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OGLES3_DIR)/libc_shim.o: amiga/libc_shim.c | $(OGLES3_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(WARP3D_DIR) $(AGL_DIR) $(MINIGL_DIR) $(OGLES2_DIR) $(OGLES3_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
