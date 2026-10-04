# Host side of QuarkTex: QuarkTex.alib, a Windows DLL loaded by WinUAE.
#
# Visual Studio 2022:  cmake -S . -B build/host -A Win32
#                      cmake --build build/host --config Release
# MinGW (Docker):      see build.sh
#
# The Amiga libraries are built separately with amiga/Makefile.

cmake_minimum_required(VERSION 3.16)
project(QuarkTex LANGUAGES CXX)

# The 68k side hands 32-bit Amiga addresses and register values straight to
# the host functions, which only works in a 32-bit process (32-bit WinUAE).
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "QuarkTex.alib must be built as a 32-bit (x86) DLL. "
		"Use -A Win32 with Visual Studio or the i686 MinGW toolchain.")
endif()

find_package(OpenGL REQUIRED)

add_library(QuarkTex SHARED QuarkTex.cpp)
target_link_libraries(QuarkTex PRIVATE OpenGL::GL OpenGL::GLU)
set_target_properties(QuarkTex PROPERTIES
	PREFIX ""
	SUFFIX ".alib"
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON)

if(MSVC)
	target_compile_definitions(QuarkTex PRIVATE _CRT_SECURE_NO_WARNINGS)
	# No dependency on the Visual C++ runtime DLLs.
	set_property(TARGET QuarkTex PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MINGW)
	# No dependency on the MinGW runtime DLLs.
	target_link_options(QuarkTex PRIVATE -static-libgcc -static-libstdc++ -static
		$<$<CONFIG:Release,MinSizeRel>:-s>)
endif()
