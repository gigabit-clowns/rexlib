# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches the Boost of release TAG, checks it against SHA256 and builds it
# with the project, static, as Boost::filesystem and Boost::headers.
#
#   rexlib_fetch_boost(TAG <tag> SHA256 <hash>)
#
# The archive is the one Boost attaches to each release for CMake: the
# archive of a commit lacks the libraries, which are submodules. Only the
# libraries rexlib includes are built, with the ones they need in turn.
# Boost::headers, which an installed Boost defines for every library that is
# only headers, holds them only when the compatibility targets are asked for.
# Container is built as headers alone, which leaves out what is not used of
# it, its allocators and memory resources, so that it is among them too.
function(rexlib_fetch_boost)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "TAG;SHA256" "")

	set(BOOST_INCLUDE_LIBRARIES container container_hash filesystem)
	set(BOOST_ENABLE_COMPATIBILITY_TARGETS ON)
	set(BOOST_CONTAINER_HEADER_ONLY ON)
	set(BUILD_SHARED_LIBS OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	set(releases "https://github.com/boostorg/boost/releases/download")
	FetchContent_Declare(
		Boost
		URL "${releases}/${arg_TAG}/${arg_TAG}-cmake.tar.xz"
		URL_HASH SHA256=${arg_SHA256}
	)
	FetchContent_MakeAvailable(Boost)
endfunction()
