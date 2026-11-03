# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.18)

# What a fetched archive holds takes the time it is extracted at, so that a
# new archive always rebuilds what came from it.
if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
	rexlib-python
	VERSION ${PROJECT_VERSION}
	DESCRIPTION "Python binding for the rexlib framework"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CTest)
# Names the library directory this platform uses, which the package
# reproduces and the extension's RPATH points at.
include(GNUInstallDirs)
include(CMakeDependentOption)
include(rexlib_python_enable_warnings)
include(rexlib_python_stubs)

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

# Its own option rather than BUILD_TESTING, nor one that depends on it:
# dependencies of dependencies write that one into the cache, and this
# project's tests should not be at their mercy.
option(
	REXLIB_PYTHON_BUILD_TESTING
	"Build the plugins the tests of the package load"
	${BUILD_TESTING}
)

# scikit-build-core stages each wheel in a directory of its own, which this
# build is told anew every time it is configured; a plain build installs
# into the interpreter that CMake found.
if(SKBUILD_PLATLIB_DIR)
	set(REXLIB_PYTHON_INSTALL_DIR "${SKBUILD_PLATLIB_DIR}")
else()
	set(
		REXLIB_PYTHON_INSTALL_DIR "${Python_SITEARCH}"
		CACHE PATH "Directory the Python package is installed into"
	)
endif()

set(
	REXLIB_PYTHON_STUBS_DIR ""
	CACHE PATH
	"Directory of pre-generated .pyi stubs. Empty generates them, which \
requires importing the extension this build produces"
)

set(
	REXLIB_PYTHON_REXLIB_VERSION 0.1
	CACHE STRING "Version of rexlib this binding requires"
)

# A prefix is copied in whole, so it must hold rexlib and nothing else, and
# no symlinks - a wheel stores those as another copy of the file. rexlib's
# rexlib_runtime and rexlib_development components produce exactly that.
set(
	REXLIB_PYTHON_STAGE_PREFIX ""
	CACHE PATH
	"rexlib installation to carry inside the package. Empty ships none, \
leaving the package to find rexlib on the system"
)

option(
	REXLIB_PYTHON_INSTALL
	"Generate the install rules of the Python package"
	ON
)

# Every dependency but rexlib is private to the extension, so building one
# here or finding an installed one changes nothing for whoever imports it.
# Turning the global switch off turns every one of the others off, on every
# configure. nanobind is neither: pip provides it.
option(
	REXLIB_PYTHON_BUILD_DEPENDENCIES
	"Build the dependencies with the binding, rather than finding installed \
ones"
	ON
)
cmake_dependent_option(
	REXLIB_PYTHON_BUILD_DLPACK "Fetch DLPack and build it with the binding" ON
	"REXLIB_PYTHON_BUILD_DEPENDENCIES" OFF
)

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

if(REXLIB_PYTHON_BUILD_DLPACK)
	include(rexlib_python_fetch_dlpack)
	rexlib_python_fetch_dlpack(
		COMMIT 84d107bf416c6bab9ae68ad285876600d230490d
	) # v1.3
else()
	# No version is asked for: the package of DLPack reports 0.6 whatever
	# the release, so asking for the 1.0 the versioned tensors arrived in
	# would refuse every release that has them.
	find_package(dlpack REQUIRED CONFIG)
endif()

if(NOT TARGET rexlib::rexlib)
	find_package(rexlib ${REXLIB_PYTHON_REXLIB_VERSION} QUIET)
endif()

# A source distribution carries rexlib's own source, which is what makes it
# installable without a rexlib to find. It installs into this build's prefix,
# which for a wheel is the package directory, so a wheel built from source
# has the same layout as one built against an installed rexlib.
if(NOT TARGET rexlib::rexlib)
	if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/rexlib/CMakeLists.txt)
		message(FATAL_ERROR
			"No rexlib found, and external/rexlib is empty. Install rexlib "
			"and point rexlib_ROOT at it, or run:\n"
			"  git submodule update --init --recursive"
		)
	endif()

	message(STATUS "No rexlib found; building the bundled one")
	add_subdirectory(external/rexlib)
else()
	message(STATUS "Using rexlib from ${rexlib_DIR}")
endif()

set(REXLIB_PYTHON_PACKAGE_DIR "${REXLIB_PYTHON_INSTALL_DIR}/rexlib")

add_subdirectory(src)

if(REXLIB_PYTHON_BUILD_TESTING)
	add_subdirectory(tests)
endif()

if(REXLIB_PYTHON_INSTALL)
	install(
		DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/rexlib/
		DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}"
		PATTERN "__pycache__" EXCLUDE
	)

	# Carries a rexlib installation inside the package, so the wheel is
	# self-contained. Mirroring the prefix keeps rexlib's own layout, which its
	# package config resolves against and beside which plugins are discovered.
	if(REXLIB_PYTHON_STAGE_PREFIX)
		install(
			DIRECTORY "${REXLIB_PYTHON_STAGE_PREFIX}/"
			DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}"
		)
	endif()

	# After everything the import needs is in place.
	rexlib_python_install_stubs(
		MODULE rexlib._binding
		DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}/_binding"
		SEARCH_PATH "${REXLIB_PYTHON_INSTALL_DIR}"
		STUBS_DIR "${REXLIB_PYTHON_STUBS_DIR}"
		GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_stubs.py"
		STUBGEN "${NB_DIR}/stubgen.py"
	)
endif()
