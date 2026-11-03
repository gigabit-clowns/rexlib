# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.18)

# What a fetched archive holds takes the time it is extracted at, so that a
# new archive always rebuilds what came from it.
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()

# The version and the CMake modules are the library's, whether the binding
# is built inside the library's build or alone against an installed rexlib.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../VERSION REXLIB_PYTHON_VERSION)
string(STRIP "${REXLIB_PYTHON_VERSION}" REXLIB_PYTHON_VERSION)
project(
	rexlib-python
	VERSION ${REXLIB_PYTHON_VERSION}
	DESCRIPTION "Python binding of rexlib"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)
list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)

include(CTest)
include(GNUInstallDirs)
include(CMakeDependentOption)
include(rexlib_enable_warnings)

# The library's own options, which inside its build are already set.
option(REXLIB_INSTALL "Generate the install rules" ON)
option(
	REXLIB_WARNINGS_AS_ERRORS
	"Fail the build of rexlib's own targets on a compiler warning"
	OFF
)
cmake_dependent_option(
	REXLIB_BUILD_TESTING "Build rexlib's tests" ON
	"BUILD_TESTING" OFF
)
option(
	REXLIB_USE_SYSTEM_DEPENDENCIES
	"Find every dependency installed, rather than fetching it and building it \
with rexlib"
	OFF
)

cmake_dependent_option(
	REXLIB_USE_SYSTEM_DLPACK
	"Find an installed DLPack rather than fetching it"
	OFF
	"NOT REXLIB_USE_SYSTEM_DEPENDENCIES"
	ON
)
option(
	REXLIB_PYTHON_STABLE_ABI
	"Build the extension against the stable ABI, so that one build serves \
Python 3.12 and newer. Needs CMake 3.26"
	OFF
)

set(REXLIB_PYTHON_COMPONENTS Interpreter Development.Module)
if(REXLIB_PYTHON_STABLE_ABI)
	if(CMAKE_VERSION VERSION_LESS 3.26)
		message(FATAL_ERROR
			"REXLIB_PYTHON_STABLE_ABI needs CMake 3.26 or newer, the first "
			"whose FindPython has the Development.SABIModule component"
		)
	endif()
	list(APPEND REXLIB_PYTHON_COMPONENTS Development.SABIModule)
endif()
find_package(Python REQUIRED COMPONENTS ${REXLIB_PYTHON_COMPONENTS})

# A wheel's install prefix is the package directory, under the platform
# library directory scikit-build-core names. Otherwise the package goes
# where the interpreter's install scheme puts it, relative to the prefix.
if(SKBUILD_PLATLIB_DIR)
	set(REXLIB_PYTHON_INSTALL_DIR "${SKBUILD_PLATLIB_DIR}")
else()
	string(
		CONCAT REXLIB_PYTHON_PLATLIB_QUERY
		"import sysconfig; "
		"print(sysconfig.get_path("
		"'platlib', vars={'base': '', 'platbase': ''}))"
	)
	execute_process(
		COMMAND "${Python_EXECUTABLE}" -c "${REXLIB_PYTHON_PLATLIB_QUERY}"
		OUTPUT_VARIABLE REXLIB_PYTHON_INSTALL_DIR_DEFAULT
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE REXLIB_PYTHON_PLATLIB_RESULT
	)
	if(NOT REXLIB_PYTHON_PLATLIB_RESULT EQUAL 0)
		message(FATAL_ERROR
			"${Python_EXECUTABLE} did not name its platform library directory"
		)
	endif()
	file(
		TO_CMAKE_PATH
		"${REXLIB_PYTHON_INSTALL_DIR_DEFAULT}"
		REXLIB_PYTHON_INSTALL_DIR_DEFAULT
	)
	string(
		REGEX REPLACE "^/+" ""
		REXLIB_PYTHON_INSTALL_DIR_DEFAULT
		"${REXLIB_PYTHON_INSTALL_DIR_DEFAULT}"
	)
	set(
		REXLIB_PYTHON_INSTALL_DIR "${REXLIB_PYTHON_INSTALL_DIR_DEFAULT}"
		CACHE STRING
		"Directory the Python package is installed into, relative to the \
install prefix"
	)
	unset(REXLIB_PYTHON_PLATLIB_QUERY)
	unset(REXLIB_PYTHON_PLATLIB_RESULT)
	unset(REXLIB_PYTHON_INSTALL_DIR_DEFAULT)
endif()
set(REXLIB_PYTHON_PACKAGE_DIR "${REXLIB_PYTHON_INSTALL_DIR}/rexlib")

# The install prefix as seen from the package, which the extension's RPATH
# is written from.
get_filename_component(
	REXLIB_PYTHON_PACKAGE_PATH
	"${REXLIB_PYTHON_PACKAGE_DIR}"
	ABSOLUTE
	BASE_DIR "${CMAKE_INSTALL_PREFIX}"
)
file(
	RELATIVE_PATH REXLIB_PYTHON_PREFIX_FROM_PACKAGE
	"${REXLIB_PYTHON_PACKAGE_PATH}"
	"${CMAKE_INSTALL_PREFIX}"
)
string(
	REGEX REPLACE "/$" ""
	REXLIB_PYTHON_PREFIX_FROM_PACKAGE
	"${REXLIB_PYTHON_PREFIX_FROM_PACKAGE}"
)
if(REXLIB_PYTHON_PREFIX_FROM_PACKAGE STREQUAL "")
	set(REXLIB_PYTHON_PREFIX_FROM_PACKAGE ".")
endif()

# The binding is C++20. The standard is required rather than preferred, so
# that nothing newer gets in unnoticed.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# nanobind ships its CMake package inside its Python package, so the
# interpreter that has it is asked where that is.
if(NOT nanobind_ROOT AND NOT nanobind_DIR)
	execute_process(
		COMMAND ${Python_EXECUTABLE} -m nanobind --cmake_dir
		OUTPUT_VARIABLE nanobind_ROOT
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
endif()
find_package(nanobind 3.0 REQUIRED CONFIG)

if(REXLIB_USE_SYSTEM_DLPACK)
	# No version is asked for: the package of DLPack reports 0.6 whatever
	# the release, so asking for the 1.0 the versioned tensors arrived in
	# would refuse every release that has them.
	find_package(dlpack REQUIRED CONFIG)
else()
	include(rexlib_python_fetch_dlpack)
	rexlib_python_fetch_dlpack(
		COMMIT 84d107bf416c6bab9ae68ad285876600d230490d
	) # v1.3
endif()

if(NOT TARGET rexlib::rexlib)
	find_package(rexlib ${PROJECT_VERSION} REQUIRED)
endif()

add_subdirectory(src)

if(REXLIB_INSTALL)
	install(
		DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rexlib/
		DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}"
		COMPONENT rexlib_python
		PATTERN "__pycache__" EXCLUDE
	)

	# Last, as generating the stubs imports the installed package. Relative
	# paths are taken from the install prefix.
	nanobind_add_stub(
		rexlib-python-stubs
		INSTALL_TIME
		RECURSIVE
		MODULE rexlib._binding
		OUTPUT_PATH "${REXLIB_PYTHON_PACKAGE_DIR}"
		PYTHON_PATH "${REXLIB_PYTHON_INSTALL_DIR}"
		COMPONENT rexlib_python
	)
endif()

if(REXLIB_BUILD_TESTING)
	add_subdirectory(
		${CMAKE_CURRENT_SOURCE_DIR}/../tests/python
		${CMAKE_CURRENT_BINARY_DIR}/tests
	)
endif()
