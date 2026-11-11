# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Builds the Catch2 in SOURCE_DIR with the project, as
# Catch2::Catch2WithMain.
#
#   rexlib_add_catch2(SOURCE_DIR <dir>)
#
# Catch2 adds the directory of its Catch module, which defines
# catch_discover_tests(), to CMAKE_MODULE_PATH, as its installed package does
# for whoever finds it, and that is handed on to the caller.
function(rexlib_add_catch2)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		Catch2
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK CMakeLists.txt
	)

	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endfunction()
