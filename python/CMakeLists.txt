cmake_minimum_required(VERSION 3.18)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
	rexlib-python
	VERSION ${PROJECT_VERSION}
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CTest)
# Names the library directory this platform uses, which the package
# reproduces and the extension's RPATH points at.
include(GNUInstallDirs)
include(python_stubs)

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

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

# Its own option rather than BUILD_TESTING: dependencies of dependencies
# write that one into the cache, and this project's tests should not be at
# their mercy.
option(REXLIB_PYTHON_BUILD_TESTING "Build this project's tests" ${BUILD_TESTING})

# scikit-build-core stages each wheel in a directory of its own, which this
# build is told anew every time it is configured; a plain build installs
# into the interpreter that CMake found.
if(SKBUILD_PLATLIB_DIR)
	set(REXLIB_PYTHON_INSTALL_DIR "${SKBUILD_PLATLIB_DIR}")
else()
	set(REXLIB_PYTHON_INSTALL_DIR "${Python_SITEARCH}" CACHE PATH
		"Directory the Python package is installed into")
endif()

set(REXLIB_PYTHON_STUBS_DIR "" CACHE PATH
	"Directory of pre-generated .pyi stubs. Empty generates them, which \
requires importing the extension this build produces")

set(REXLIB_PYTHON_REXLIB_VERSION 0.1 CACHE STRING
	"Version of rexlib this binding requires")

# A prefix is copied in whole, so it must hold rexlib and nothing else, and
# no symlinks - a wheel stores those as another copy of the file. rexlib's
# rexlib_runtime and rexlib_development components produce exactly that.
set(REXLIB_PYTHON_STAGE_PREFIX "" CACHE PATH
	"rexlib installation to carry inside the package. Empty ships none, \
leaving the package to find rexlib on the system")

option(REXLIB_PYTHON_INSTALL "Generate install rules for the Python package" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
	nanobind
	GIT_REPOSITORY https://github.com/wjakob/nanobind
	GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(nanobind)

# Only the header is wanted. SOURCE_SUBDIR names a directory dlpack does not
# have, so that its own build file, which installs itself and builds a mock,
# is not read.
FetchContent_Declare(
	dlpack
	GIT_REPOSITORY https://github.com/dmlc/dlpack
	GIT_TAG v1.3
	SOURCE_SUBDIR header-only
)
FetchContent_MakeAvailable(dlpack)

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

# ---------------------------------------------------------------------------
# The package
# ---------------------------------------------------------------------------

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
	install_python_stubs(
		MODULE rexlib._binding
		DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}/_binding"
		SEARCH_PATH "${REXLIB_PYTHON_INSTALL_DIR}"
		STUBS_DIR "${REXLIB_PYTHON_STUBS_DIR}"
		GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_stubs.py"
		STUBGEN "${nanobind_SOURCE_DIR}/src/stubgen.py"
	)
endif()
