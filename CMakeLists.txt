# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.18)

# Without this CMAKE_MSVC_DEBUG_INFORMATION_FORMAT is silently ignored.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
endif()

# What a fetched archive holds takes the time it is extracted at, so that a
# new archive always rebuilds what came from it.
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
	rexlib
	VERSION ${PROJECT_VERSION}
	DESCRIPTION
		"Device-agnostic array computing library for CryoEM and CryoET image \
processing, with a pluggable backend architecture"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(rexlib_enable_warnings)

option(REXLIB_INSTALL "Generate rexlib's install rules" ON)
option(
	REXLIB_BUILD_DOCS
	"Add the target that builds the documentation of the API, which needs \
Doxygen"
	OFF
)
option(
	REXLIB_ENABLE_COVERAGE
	"Instrument the library and the tests for code coverage"
	OFF
)
option(
	REXLIB_REGISTER_TESTS_PER_BINARY
	"Register one test per test executable instead of one per Catch2 case"
	OFF
)
# Only while BUILD_TESTING, the switch of CTest, is on: tests built without
# it could not be registered with CTest. Its own option lets a project that
# embeds rexlib drop rexlib's tests without dropping its own.
cmake_dependent_option(
	REXLIB_BUILD_TESTING "Build rexlib's tests" ON
	"BUILD_TESTING" OFF
)

# Every dependency is private and none appears in a public header, so building
# one here or finding an installed one changes nothing for a consumer of the
# installed library. Turning the global switch off turns every one of the
# others off, on every configure.
option(
	REXLIB_BUILD_DEPENDENCIES
	"Build the dependencies with rexlib, rather than finding installed ones"
	ON
)
cmake_dependent_option(
	REXLIB_BUILD_BOOST "Fetch Boost and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_BUILD_SPDLOG "Fetch spdlog and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_BUILD_HALF "Fetch half and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_BUILD_POCKETFFT "Fetch pocketfft and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_BUILD_EIGEN "Fetch Eigen and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)

# The default goes through a variable of its own: assigning to the cache
# entry's own name first would leave a normal variable shadowing it, and the
# value given on the command line would be overwritten on every configure.
#
# STRING rather than PATH, because this is a destination relative to the
# install prefix: cmake turns a relative -D value of a PATH entry into an
# absolute one, which would place the plugins outside the prefix.
if(WIN32)
	set(REXLIB_PLUGINS_DIR_DEFAULT ${CMAKE_INSTALL_BINDIR}/rexlib-plugins)
else()
	set(REXLIB_PLUGINS_DIR_DEFAULT ${CMAKE_INSTALL_LIBDIR}/rexlib-plugins)
endif()
set(
	REXLIB_PLUGINS_INSTALL_DIR ${REXLIB_PLUGINS_DIR_DEFAULT}
	CACHE STRING "Directory where the rexlib plugins are installed"
)
unset(REXLIB_PLUGINS_DIR_DEFAULT)

# Windows looks for a library beside the executable that needs it, and has no
# other way to be told where one is. So the library and the test executables
# are all built into one directory.
if(WIN32 AND NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

find_package(Threads REQUIRED)

if(REXLIB_BUILD_BOOST)
	include(rexlib_fetch_boost)
	rexlib_fetch_boost(
		TAG boost-1.92.0
		SHA256 9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
	)
else()
	# CONFIG mode, because only the BoostConfig.cmake that Boost has
	# installed since 1.70 defines Boost::headers.
	find_package(Boost 1.70 REQUIRED CONFIG COMPONENTS filesystem)
endif()

if(REXLIB_BUILD_HALF)
	include(rexlib_fetch_half)
	rexlib_fetch_half(
		VERSION 2.2.1
		SHA256 76ddbf406e9d9b772ec73af2bf925b38b290b4390cc4064720a08d4b4bca0aa9
	)
else()
	# The header carries no version, so none is asked for.
	find_package(half REQUIRED MODULE)
endif()

if(REXLIB_BUILD_SPDLOG)
	include(rexlib_fetch_spdlog)
	rexlib_fetch_spdlog(
		COMMIT 79524ddd08a4ec981b7fea76afd08ee05f83755d
	) # v1.17.0
else()
	find_package(spdlog 1.4 REQUIRED CONFIG)
endif()

if(REXLIB_BUILD_POCKETFFT)
	include(rexlib_fetch_pocketfft)
	rexlib_fetch_pocketfft(
		COMMIT 4d865cba24359b05b37da4e75aee24498c3b78e7
	) # cpp
else()
	# pocketfft makes no releases, so no version is asked for.
	find_package(pocketfft REQUIRED MODULE)
endif()

if(REXLIB_BUILD_EIGEN)
	include(rexlib_fetch_eigen)
	rexlib_fetch_eigen(
		COMMIT bc3b39870ecb690a623a3f49149a358b95c5781d
	) # 5.0.1
else()
	# No version is asked for: the package of Eigen takes one for a major
	# version to match rather than for a floor, so asking for the 3.3 every
	# distribution carries would refuse the 5 that is fetched.
	find_package(Eigen3 REQUIRED CONFIG)
endif()

# Same helper the installed package config offers, so that a plugin built
# inside a superbuild installs itself exactly as an out-of-tree one does.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/rexlib-plugin.cmake)

# Project-wide default for every target created below. The sources are
# written to C++14 and reach anything newer through the REXLIB_* feature
# macros, so this is a preference and not a requirement: a compiler without
# C++17 still builds them.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

if(REXLIB_BUILD_TESTING)
	add_subdirectory(tests)
endif()

if(REXLIB_BUILD_DOCS)
	add_subdirectory(doc)
endif()
