include(FetchContent)

# Makes ZLIB::ZLIB available, either from the system or by fetching and
# building it. See rexlib_add_boost() on why VERSION and MINIMUM differ.
#
# zlib is here for libtiff alone, which needs it to read and write Deflate
# compressed files, so nothing is done when libtiff comes from the system: a
# packaged libtiff already carries the codecs it was built with.
function(rexlib_add_zlib)
	set(options)
	set(oneValueArgs VERSION MINIMUM)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	if(REXLIB_USE_SYSTEM_TIFF)
		return()
	endif()

	if(REXLIB_USE_SYSTEM_ZLIB)
		find_package(ZLIB ${arg_MINIMUM} REQUIRED)
		return()
	endif()

	if(POLICY CMP0135)
		cmake_policy(SET CMP0135 NEW) # To avoid warnings
	endif()

	# zlib is linked statically and privately, so nothing of it belongs in
	# an install of this project.
	set(ZLIB_INSTALL OFF)
	set(ZLIB_BUILD_SHARED OFF)
	set(ZLIB_BUILD_STATIC ON)
	set(ZLIB_BUILD_TESTING OFF)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	set(archive "https://github.com/madler/zlib/archive/refs/tags")
	FetchContent_Declare(zlib URL "${archive}/v${arg_VERSION}.tar.gz")
	FetchContent_MakeAvailable(zlib)

	# zlib names its static library ZLIB::ZLIBSTATIC and keeps ZLIB::ZLIB
	# for the shared one, which is not built here. ZLIB::ZLIB is the name
	# find_package() provides, so it is the one everything links.
	if(NOT TARGET ZLIB::ZLIB)
		add_library(ZLIB::ZLIB ALIAS zlibstatic)
	endif()

	# What makes the find_package(ZLIB) of a project fetched after this one
	# settle on the target above instead of searching the system: FindZLIB
	# searches for neither variable when it is already set, and defines no
	# target when there already is one.
	set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" PARENT_SCOPE)
	set(ZLIB_LIBRARY ZLIB::ZLIB PARENT_SCOPE)
endfunction()
