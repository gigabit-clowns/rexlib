include(FetchContent)

# Makes TIFF::TIFF available, either from the system or by fetching and
# building libtiff. See rexlib_add_boost() on why VERSION and MINIMUM differ.
#
# rexlib_add_zlib() has to be called first: the fetched libtiff takes its
# Deflate codec from the zlib that call provides.
function(rexlib_add_tiff)
	set(options)
	set(oneValueArgs VERSION MINIMUM)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	if(REXLIB_USE_SYSTEM_TIFF)
		find_package(TIFF ${arg_MINIMUM} REQUIRED)
		return()
	endif()

	if(POLICY CMP0135)
		cmake_policy(SET CMP0135 NEW) # To avoid warnings
	endif()

	# libtiff asks for the policies of CMake 3.10, under which option()
	# discards a normal variable of the same name, and with it everything
	# set below.
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	# libtiff is linked statically and privately, so nothing of it belongs
	# in an install of this project, and only the library is built.
	set(BUILD_SHARED_LIBS OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(tiff-install OFF)
	set(tiff-tools OFF)
	set(tiff-tests OFF)
	set(tiff-contrib OFF)
	set(tiff-docs OFF)
	set(tiff-cxx OFF)

	# libtiff enables every codec whose library it finds on the machine,
	# which would make what a build can read depend on where it was built
	# and would link libraries the installed package does not carry. zlib is
	# the one this project provides, so it is the one left on.
	set(zlib ON)
	set(libdeflate OFF)
	set(jbig OFF)
	set(jpeg OFF)
	set(old-jpeg OFF)
	set(lerc OFF)
	set(lzma OFF)
	set(webp OFF)
	set(zstd OFF)

	set(archive "https://gitlab.com/libtiff/libtiff/-/archive")
	set(url "${archive}/v${arg_VERSION}/libtiff-v${arg_VERSION}.tar.gz")
	FetchContent_Declare(tiff URL "${url}")
	FetchContent_MakeAvailable(tiff)

	# libtiff names its target TIFF::tiff, whereas find_package() provides
	# TIFF::TIFF, which is the one this project links.
	if(NOT TARGET TIFF::TIFF)
		add_library(TIFF::TIFF ALIAS tiff)
	endif()
endfunction()
