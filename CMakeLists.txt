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
	REXLIB_DOCS_USE_README_AS_MAINPAGE
	"Use README.md as the Doxygen main page when building documentation"
	ON
)
option(
	REXLIB_DOCS_STRIP_BADGES_FROM_MAINPAGE
	"Strip badge blocks from the README copy used as Doxygen main page"
	ON
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
option(
	REXLIB_CHECK_HEADERS
	"Compile every public header on its own under each C++ standard the \
compiler supports"
	OFF
)
option(
	REXLIB_WARNINGS_AS_ERRORS
	"Fail the build of rexlib's own targets on a compiler warning"
	OFF
)
option(
	REXLIB_CLANG_TIDY
	"Run clang-tidy on rexlib's own sources as they are compiled, failing \
the build on any finding"
	OFF
)
option(
	REXLIB_CHECK_STYLE
	"Test every file git tracks against .editorconfig, and the Python files \
with ruff, which need editorconfig-checker and ruff"
	OFF
)
option(
	REXLIB_BUILD_PYTHON
	"Build the Python binding, which needs Python and nanobind"
	OFF
)
# Only while BUILD_TESTING, the switch of CTest, is on: tests built without
# it could not be registered with CTest. Its own option lets a project that
# embeds rexlib drop rexlib's tests without dropping its own.
cmake_dependent_option(
	REXLIB_BUILD_TESTING "Build rexlib's tests" ON
	"BUILD_TESTING" OFF
)
# The benchmarks use the Catch2 the tests fetch or find.
cmake_dependent_option(
	REXLIB_BUILD_BENCHMARKS "Build rexlib's benchmarks" OFF
	"REXLIB_BUILD_TESTING" OFF
)

# Every dependency is private and none appears in a public header, so building
# one here or finding an installed one changes nothing for a consumer of the
# installed library. Turning the global switch on turns every one of the
# others on, on every configure.
option(
	REXLIB_USE_SYSTEM_DEPENDENCIES
	"Find every dependency installed, rather than fetching it and building it \
with rexlib"
	OFF
)
cmake_dependent_option(
	REXLIB_USE_SYSTEM_BOOST
	"Find an installed Boost rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_USE_SYSTEM_SPDLOG
	"Find an installed spdlog rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_USE_SYSTEM_HALF
	"Find an installed half rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_USE_SYSTEM_POCKETFFT
	"Find an installed pocketfft rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_USE_SYSTEM_EIGEN
	"Find an installed Eigen rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
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

# Where the public headers CMake writes, such as the version, go.
set(REXLIB_GENERATED_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)

find_package(Threads REQUIRED)

if(REXLIB_USE_SYSTEM_BOOST)
	# CONFIG mode, because only the BoostConfig.cmake that Boost has
	# installed since 1.70 defines Boost::headers.
	find_package(Boost 1.70 REQUIRED CONFIG COMPONENTS filesystem)
else()
	include(rexlib_fetch_boost)
	rexlib_fetch_boost(
		TAG boost-1.92.0
		SHA256 9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
	)
endif()

if(REXLIB_USE_SYSTEM_HALF)
	# The header carries no version, so none is asked for.
	find_package(half REQUIRED MODULE)
else()
	include(rexlib_fetch_half)
	rexlib_fetch_half(
		VERSION 2.2.1
		SHA256 76ddbf406e9d9b772ec73af2bf925b38b290b4390cc4064720a08d4b4bca0aa9
	)
endif()

if(REXLIB_USE_SYSTEM_SPDLOG)
	find_package(spdlog 1.4 REQUIRED CONFIG)
else()
	include(rexlib_fetch_spdlog)
	rexlib_fetch_spdlog(
		COMMIT 79524ddd08a4ec981b7fea76afd08ee05f83755d
	) # v1.17.0
endif()

if(REXLIB_USE_SYSTEM_POCKETFFT)
	# pocketfft makes no releases, so no version is asked for.
	find_package(pocketfft REQUIRED MODULE)
else()
	include(rexlib_fetch_pocketfft)
	rexlib_fetch_pocketfft(
		COMMIT 4d865cba24359b05b37da4e75aee24498c3b78e7
	) # cpp
endif()

if(REXLIB_USE_SYSTEM_EIGEN)
	# No version is asked for: the package of Eigen takes one for a major
	# version to match rather than for a floor, so asking for the 3.3 every
	# distribution carries would refuse the 5 that is fetched.
	find_package(Eigen3 REQUIRED CONFIG)
else()
	include(rexlib_fetch_eigen)
	rexlib_fetch_eigen(
		COMMIT bc3b39870ecb690a623a3f49149a358b95c5781d
	) # 5.0.1
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

if(REXLIB_CLANG_TIDY)
	find_program(REXLIB_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

add_subdirectory(src)

# After src, so that the library is installed before the binding, whose
# stubs are generated by importing it.
if(REXLIB_BUILD_PYTHON)
	add_subdirectory(python)
endif()

if(REXLIB_BUILD_TESTING)
	add_subdirectory(tests)
endif()

if(REXLIB_BUILD_BENCHMARKS)
	add_subdirectory(benchmarks)
endif()

if(REXLIB_CHECK_HEADERS)
	add_subdirectory(tests/headers)
endif()

if(REXLIB_CHECK_STYLE)
	add_subdirectory(tests/style)
endif()

if(REXLIB_BUILD_DOCS)
	add_subdirectory(docs)
endif()

# The tests a memory check leaves out, which CTest reads from this file of
# the build tree.
get_property(REXLIB_MEMCHECK_IGNORE GLOBAL PROPERTY REXLIB_MEMCHECK_IGNORE)
file(
	WRITE ${PROJECT_BINARY_DIR}/CTestCustom.cmake
	"list(APPEND CTEST_CUSTOM_MEMCHECK_IGNORE ${REXLIB_MEMCHECK_IGNORE})\n"
)
