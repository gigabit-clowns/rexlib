# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Builds the Boost in SOURCE_DIR with the project, static, as a Boost::
# target per library and Boost::headers.
#
#   rexlib_add_boost(SOURCE_DIR <dir>)
#
# SOURCE_DIR is laid out as Boost's superproject: each library in libs/,
# Boost's CMake in tools/cmake, and the root CMakeLists.txt that builds them.
# Every library in libs/ is built, so the libraries checked out there are the
# ones there are. Boost::headers, which an installed Boost defines for every
# library that is only headers, holds them only when the compatibility
# targets are asked for. Container is built as headers alone, which leaves
# out what is not used of it, its allocators and memory resources, so that it
# is among them too.
function(rexlib_add_boost)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	set(BOOST_ENABLE_COMPATIBILITY_TARGETS ON)
	set(BOOST_CONTAINER_HEADER_ONLY ON)
	set(BUILD_SHARED_LIBS OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	rexlib_add_dependency(
		Boost
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK tools/cmake/include/BoostRoot.cmake
	)
endfunction()
