# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches half at VERSION, checks it against SHA256 and makes it available as
# half::half, as Findhalf.cmake does for an installed one.
#
#   rexlib_fetch_half(VERSION <version> SHA256 <hash>)
#
# half is a single header with no CMake of its own, so the target is defined
# here. Its literals are left out: they are not used, and Clang warns about
# the spacing of their declaration.
function(rexlib_fetch_half)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "VERSION;SHA256" "")

	set(files "https://downloads.sourceforge.net/project/half/half")
	FetchContent_Declare(
		half
		URL "${files}/${arg_VERSION}/half-${arg_VERSION}.zip"
		URL_HASH SHA256=${arg_SHA256}
	)
	FetchContent_MakeAvailable(half)

	add_library(half INTERFACE)
	add_library(half::half ALIAS half)
	target_include_directories(
		half
		SYSTEM INTERFACE
			${half_SOURCE_DIR}/include
	)
	target_compile_definitions(
		half
		INTERFACE
			HALF_ENABLE_CPP11_USER_LITERALS=0
	)
endfunction()
