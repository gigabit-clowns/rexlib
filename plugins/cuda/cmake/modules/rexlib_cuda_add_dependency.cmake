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
#
# The version of a dependency whose sources are built is kept in the global
# property REXLIB_CUDA_DEPENDENCY_VERSION_<SOURCE_DIR>: the one its own build
# writes in a package version file, or else the one its project(NAME)
# states.
function(rexlib_cuda_add_dependency name)
	cmake_parse_arguments(
		PARSE_ARGV 1 arg "" "SOURCE_DIR;CHECK;SOURCE_SUBDIR" ""
	)
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

	if(EXISTS "${arg_SOURCE_DIR}/${arg_SOURCE_SUBDIR}/CMakeLists.txt")
		string(TOLOWER "${name}" lower_name)
		file(
			GLOB_RECURSE version_file
			"${${lower_name}_BINARY_DIR}/${lower_name}-config-version.cmake"
			"${${lower_name}_BINARY_DIR}/${name}ConfigVersion.cmake"
		)
		if(version_file)
			list(GET version_file 0 version_file)
			include("${version_file}")
			set(version "${PACKAGE_VERSION}")
		else()
			get_directory_property(
				version DIRECTORY "${arg_SOURCE_DIR}" DEFINITION ${name}_VERSION
			)
		endif()
		set_property(
			GLOBAL PROPERTY REXLIB_CUDA_DEPENDENCY_VERSION_${arg_SOURCE_DIR} "${version}"
		)
	endif()

	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endfunction()
