# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches Catch2 at COMMIT, that of a release, and builds it with the
# project, as Catch2::Catch2WithMain.
#
#   rexlib_fetch_catch2(COMMIT <sha>)
#
# Catch2 adds the directory of its Catch module, which defines
# catch_discover_tests(), to CMAKE_MODULE_PATH, as its installed package does
# for whoever finds it. That happens in this function's scope, so it is
# handed on to the caller.
function(rexlib_fetch_catch2)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	# From CMake 3.25 its headers count as system headers, whose
	# warnings are not rexlib's.
	set(system "")
	if(NOT CMAKE_VERSION VERSION_LESS 3.25)
		set(system SYSTEM)
	endif()
	FetchContent_Declare(
		Catch2
		${system}
		URL "https://github.com/catchorg/Catch2/archive/${arg_COMMIT}.tar.gz"
	)
	FetchContent_MakeAvailable(Catch2)

	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endfunction()
