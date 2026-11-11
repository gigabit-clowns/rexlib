# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Makes the half in SOURCE_DIR available as half::half, as Findhalf.cmake
# does for an installed one.
#
#   rexlib_add_half(SOURCE_DIR <dir>)
#
# half is a single header with no CMake of its own, so the target is defined
# here. Its literals are left out: they are not used, and Clang warns about
# the spacing of their declaration.
function(rexlib_add_half)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		half
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK include/half.hpp
	)

	add_library(half INTERFACE)
	add_library(half::half ALIAS half)
	target_include_directories(
		half
		SYSTEM INTERFACE
			"${arg_SOURCE_DIR}/include"
	)
	target_compile_definitions(
		half
		INTERFACE
			HALF_ENABLE_CPP11_USER_LITERALS=0
	)
endfunction()
