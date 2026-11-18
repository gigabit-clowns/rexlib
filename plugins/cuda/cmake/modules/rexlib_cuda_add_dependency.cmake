# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Adds the dependency NAME to the project from its sources in SOURCE_DIR.
#
#   rexlib_cuda_add_dependency(
#   	<name> SOURCE_DIR <dir> CHECK <file> [SOURCE_SUBDIR <subdir>]
#   )
#
# CHECK is one of the dependency's own files, relative to SOURCE_DIR. When it
# is missing, as when the git submodules were not checked out, the configure
# stops with the command to run. A SOURCE_SUBDIR that holds no
# CMakeLists.txt leaves the sources unbuilt. What the dependency adds to
# CMAKE_MODULE_PATH for its parent is handed on to the caller.
function(rexlib_cuda_add_dependency name)
	cmake_parse_arguments(PARSE_ARGV 1 arg "" "SOURCE_DIR;CHECK;SOURCE_SUBDIR" "")
	if(NOT EXISTS "${arg_SOURCE_DIR}/${arg_CHECK}")
		message(
			FATAL_ERROR
			"The sources of ${name} are missing from ${arg_SOURCE_DIR}. "
			"Check out the git submodules: git submodule update --init"
		)
	endif()

	set(options)
	if(arg_SOURCE_SUBDIR)
		list(APPEND options SOURCE_SUBDIR ${arg_SOURCE_SUBDIR})
	endif()

	FetchContent_Declare(${name} SOURCE_DIR "${arg_SOURCE_DIR}" ${options})
	FetchContent_MakeAvailable(${name})

	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endfunction()
