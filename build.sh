#!/bin/sh
# Builds QuarkTex in Docker; only Docker and a POSIX shell (Git Bash on
# Windows) are needed.
#
#   ./build.sh            Amiga libraries + host DLL, collected in dist/
#   ./build.sh amiga      Warp3D.library and agl.library only
#   ./build.sh host       QuarkTex.alib only
#   ./build.sh generate   regenerate gl/*.auto.* from gl/glFuncs.txt
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
	MSYS_NO_PATHCONV=1 docker run --rm $USER_FLAGS -v "$VOLUME:/w" -w /w "$image" sh -c "$*"
}

host_image() {
	docker build -q -t "$HOST_IMAGE" -f docker/host.Dockerfile docker >/dev/null
}

generate() {
	host_image
	run "$HOST_IMAGE" "mkdir -p build && g++ -std=c++98 -Wall -O2 -o build/glgen gl/main.cpp && build/glgen gl"
	# The generated files are stored with CRLF line endings.
	for f in gl/glstatichandles.auto.c gl/glDLLfunc.auto.c gl/gldefinitions.auto.c gl/gldeclarations.auto.h; do
		sed -i 's/\r*$/\r/' "$f"
	done
}

amiga() {
	run "$AMIGA_IMAGE" "make -f amiga/Makefile"
}

host() {
	host_image
	run "$HOST_IMAGE" "cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-i686.cmake >/dev/null && cmake --build build/host"
}

dist() {
	rm -rf dist
	mkdir -p dist/alib
	cp build/amiga/Warp3D.library build/amiga/agl.library dist/
	cp build/host/QuarkTex.alib dist/alib/
	cp License.txt ReadMe.txt dist/
	echo "dist/:"
	ls -lR dist
}

case "${1:-all}" in
	all) amiga; host; dist ;;
	amiga) amiga ;;
	host) host ;;
	generate) generate ;;
	clean) rm -rf build dist ;;
	*) echo "usage: $0 [all|amiga|host|generate|clean]" >&2; exit 1 ;;
esac
