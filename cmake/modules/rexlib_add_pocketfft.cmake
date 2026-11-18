# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Makes the pocketfft in SOURCE_DIR, a commit of its cpp branch, available as
# pocketfft::pocketfft, as Findpocketfft.cmake does for an installed one.
#
#   rexlib_add_pocketfft(SOURCE_DIR <dir>)
#
# pocketfft makes no releases, and is a single header with no CMake of its
# own, so the target is defined here.
function(rexlib_add_pocketfft)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		pocketfft
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK pocketfft_hdronly.h
	)

	add_library(pocketfft INTERFACE)
	add_library(pocketfft::pocketfft ALIAS pocketfft)
	target_include_directories(
		pocketfft
		SYSTEM INTERFACE
			"${arg_SOURCE_DIR}"
	)
endfunction()
