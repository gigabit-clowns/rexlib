# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches pocketfft at COMMIT, one of its cpp branch, and makes it available
# as pocketfft::pocketfft, as Findpocketfft.cmake does for an installed one.
#
#   rexlib_fetch_pocketfft(COMMIT <sha>)
#
# pocketfft makes no releases, and is a single header with no CMake of its
# own, so the target is defined here.
function(rexlib_fetch_pocketfft)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	set(archives "https://github.com/mreineck/pocketfft/archive")
	FetchContent_Declare(
		pocketfft
		URL "${archives}/${arg_COMMIT}.tar.gz"
	)
	FetchContent_MakeAvailable(pocketfft)

	add_library(pocketfft INTERFACE)
	add_library(pocketfft::pocketfft ALIAS pocketfft)
	target_include_directories(
		pocketfft
		SYSTEM INTERFACE
			${pocketfft_SOURCE_DIR}
	)
endfunction()
