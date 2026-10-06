# Host side of QuarkTex NG: the library the emulator loads through
# uaenative.library. The name carries the architecture, as uaenative expects:
# quarktexng-windows-x86.dll (32-bit WinUAE) or quarktexng-windows-x86-64.dll.
#
# Visual Studio 2022:  cmake -S . -B build/host-x64 -A x64     (or -A Win32)
#                      cmake --build build/host-x64 --config Release
# MinGW (Docker):      see build.sh
#
# The Amiga libraries are built separately with amiga/Makefile.

cmake_minimum_required(VERSION 3.16)
project(QuarkTex LANGUAGES CXX)

find_package(OpenGL REQUIRED)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(QT_ARCH x86-64)
else()
	set(QT_ARCH x86)
endif()

add_library(quarktex SHARED host/quarktex.cpp host/gldecode.cpp host/w3d.cpp host/mgl.cpp host/ffp.cpp host/gl3.cpp host/present.cpp
	host/gldecode.h host/uni.h host/ffp.h host/gl3.h host/present.h gl/w3dcmd.h)
target_include_directories(quarktex PRIVATE gl)
target_link_libraries(quarktex PRIVATE OpenGL::GL OpenGL::GLU)
set_target_properties(quarktex PROPERTIES
	PREFIX ""
	OUTPUT_NAME "quarktexng-windows-${QT_ARCH}"
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON)

if(MSVC)
	target_compile_definitions(quarktex PRIVATE _CRT_SECURE_NO_WARNINGS)
	# No dependency on the Visual C++ runtime DLLs.
	set_property(TARGET quarktex PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MINGW)
	# No dependency on the MinGW runtime DLLs.
	target_link_options(quarktex PRIVATE -static-libgcc -static-libstdc++ -static
		$<$<CONFIG:Release,MinSizeRel>:-s>)
endif()
