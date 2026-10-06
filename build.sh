#!/bin/sh
# Builds QuarkTex in Docker; only Docker and a POSIX shell (Git Bash on
# Windows) are needed.
#
#   ./build.sh            Amiga libraries + host DLLs, collected in dist/
#   ./build.sh amiga      Warp3D.library and agl.library only
#   ./build.sh host       quarktexng-windows-x86.dll and -x86-64.dll only
#   ./build.sh tests      Warp3D test programs and the legacy QuarkTex.alib
#                         the 0.53 reference libraries need (see tests/run.ps1)
#   ./build.sh generate   regenerate the *.auto.* files from gl/glFuncs.txt
#   ./build.sh unittest   check the generated encoder/decoder pair
#   ./build.sh aminet     the Aminet archive and readme from dist/ (after ./build.sh)
#   ./build.sh clean
set -e

cd "$(dirname "$0")"

AMIGA_IMAGE=amigadev/crosstools:m68k-amigaos
HOST_IMAGE=quarktex-host

if command -v cygpath >/dev/null 2>&1; then
	VOLUME=$(cygpath -w "$(pwd)")
	USER_FLAGS=
else
	VOLUME=$(pwd)
	USER_FLAGS="-u $(id -u):$(id -g)"
fi

run() {
	image=$1
	shift
	MSYS_NO_PATHCONV=1 docker run --rm $USER_FLAGS -v "$VOLUME:/w" $SDK_MOUNT -w /w "$image" sh -c "$*"
}

host_image() {
	docker build -q -t "$HOST_IMAGE" -f docker/host.Dockerfile docker >/dev/null
}

generate() {
	host_image
	run "$HOST_IMAGE" "mkdir -p build tests/host && g++ -std=c++11 -Wall -O2 -o build/glgen gl/glgen.cpp && build/glgen gl/glFuncs.txt ."
}

# Encoder/decoder round trip for every OpenGL function, on the build host.
unittest() {
	host_image
	run "$HOST_IMAGE" "mkdir -p build && g++ -std=c++11 -Wall -Wno-int-to-pointer-cast -O1 -Igl -o build/unittest tests/host/test.cpp && build/unittest"
}

# MiniGL's SDK include directory (from the PiStorm3D or MiniGL Classic
# archive) for minigl.library; without it that library is not built.
SDK_MOUNT=
if [ -n "$MINIGL_SDK" ] && [ -f "$MINIGL_SDK/libraries/minigl_dispatch.h" ]; then
	if command -v cygpath >/dev/null 2>&1; then SDK_MOUNT="-v $(cygpath -w "$MINIGL_SDK"):/sdk:ro"
	else SDK_MOUNT="-v $MINIGL_SDK:/sdk:ro"; fi
fi

amiga() {
	if [ -n "$SDK_MOUNT" ]; then
		host_image
		run "$HOST_IMAGE" "python3 minigl.library/mglgen.py /sdk gl build/amiga/minigl minigl.library/*.c"
	fi
	run "$AMIGA_IMAGE" "make -f amiga/Makefile"
}

# The QuarkTex 0.53 libraries from Aminet talk to QuarkTex.alib through the
# uaelib traps. For the reference runs, build that host DLL as it was at the
# end of phase 2: the 0.53 code plus frame capture.
LEGACY_HOST_COMMIT=2a6a686

legacy_host() {
	host_image
	mkdir -p build/legacy
	git show "$LEGACY_HOST_COMMIT:QuarkTex.cpp" > build/legacy/QuarkTex.cpp
	run "$HOST_IMAGE" "i686-w64-mingw32-g++ -std=c++11 -O2 -shared -static -s -o build/legacy/QuarkTex.alib build/legacy/QuarkTex.cpp -lopengl32 -lglu32 -lgdi32"
}

tests() {
	run "$AMIGA_IMAGE" "make -f tests/Makefile"
	legacy_host
}

host() {
	host_image
	run "$HOST_IMAGE" "cmake -S . -B build/host-x86 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake >/dev/null && cmake --build build/host-x86"
	run "$HOST_IMAGE" "cmake -S . -B build/host-x64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-x86_64.cmake >/dev/null && cmake --build build/host-x64"
}

dist() {
	rm -rf dist
	mkdir -p dist
	cp build/amiga/Warp3D.library build/amiga/agl.library dist/
	if [ -f build/amiga/minigl.library ]; then cp build/amiga/minigl.library dist/; fi
	cp build/host-x86/quarktexng-windows-x86.dll build/host-x64/quarktexng-windows-x86-64.dll dist/
	cp License.txt COPYING ThirdParty.txt ReadMe.txt dist/
	echo "dist/:"
	ls -lR dist
}

# The Aminet release: build/aminet/QuarkTexNG.lha and QuarkTexNG.readme
# (amiga/aminet/QuarkTexNG.readme). The archive holds the libraries and DLLs
# of dist/, the documents and licenses, and the source of the commit they
# were built from, which the LGPL asks for; so the tree must be committed.
# minigl.library must be in dist/ (MINIGL_SDK set for the build).
aminet() {
	if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
		echo "aminet: commit first, the archive carries the source of HEAD" >&2
		exit 1
	fi
	for f in Warp3D.library agl.library minigl.library quarktexng-windows-x86.dll quarktexng-windows-x86-64.dll; do
		if [ ! -f "dist/$f" ]; then echo "aminet: dist/$f missing (./build.sh with MINIGL_SDK set)" >&2; exit 1; fi
	done
	package=build/aminet/QuarkTexNG
	rm -rf build/aminet
	mkdir -p "$package/Libs" "$package/WinUAE" "$package/Source"
	cp dist/Warp3D.library dist/agl.library dist/minigl.library "$package/Libs/"
	cp dist/quarktexng-windows-x86.dll dist/quarktexng-windows-x86-64.dll "$package/WinUAE/"
	# Text files with Amiga (LF) line ends.
	for f in ReadMe.txt README.md License.txt COPYING ThirdParty.txt amiga/aminet/QuarkTexNG.readme; do
		tr -d '\r' < "$f" > "$package/$(basename "$f")"
	done
	git archive HEAD | tar -x -C "$package/Source"
	git rev-parse HEAD > "$package/Source/COMMIT"
	cp "$package/QuarkTexNG.readme" build/aminet/QuarkTexNG.readme
	run "$AMIGA_IMAGE" "cd build/aminet && lha ao5q QuarkTexNG.lha QuarkTexNG && lha t QuarkTexNG.lha >/dev/null"
	echo "build/aminet/:"
	ls -l build/aminet
}

case "${1:-all}" in
	all) amiga; host; dist ;;
	aminet) aminet ;;
	amiga) amiga ;;
	host) host ;;
	tests) tests ;;
	generate) generate ;;
	unittest) unittest ;;
	clean) rm -rf build dist ;;
	*) echo "usage: $0 [all|amiga|host|tests|generate|unittest|aminet|clean]" >&2; exit 1 ;;
esac
