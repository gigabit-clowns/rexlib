cmake_minimum_required(VERSION 3.18)

# Define the project
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
  xmipp4-python
	VERSION ${PROJECT_VERSION}
	LANGUAGES C CXX
)

# Include CTest to provide BUILD_TESTING option
include(CTest)
include(FetchContent)
include(GNUInstallDirs)

# Declare build options
#option(XMIPP4_PYTHON_BUILD_DOC "Build documentation" ON)

# Set the module path
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

# Obtain pybind11
FetchContent_Declare(
	pybind11
	GIT_REPOSITORY https://github.com/pybind/pybind11
	GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(pybind11)

# Find xmipp4-core
if (NOT TARGET xmipp4-core)
	find_package(xmipp4-core REQUIRED)
endif()

if(SKBUILD_PLATLIB_DIR)
	set(PYTHON_INSTALL_DIR "${SKBUILD_PLATLIB_DIR}")
else()
	set(PYTHON_INSTALL_DIR "${Python_SITEARCH}")
endif()

# Set project-wide compilation options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Add sources
add_subdirectory(src)

# Only build docs if it is the main project
#if(BUILD_DOC)
#	add_subdirectory(doc)
#endif()

# Only build tests if it is the main project
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

# Install python directory
install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/python/xmipp4/
	DESTINATION "${PYTHON_INSTALL_DIR}/xmipp4"
)

# Type stubs are generated from the extension that was just installed, so
# this has to come after everything the import needs is in place.
#
# Generating them requires importing the extension, which a cross-compiling
# build cannot do. Those builds get stubs handed to them through
# XMIPP4_STUBS_DIR instead; the stubs describe the Python API, which is the
# same whatever the target architecture.
set(XMIPP4_STUBS_DIR "" CACHE PATH
	"Directory holding pre-generated stubs, for builds that cannot import \
the extension they produce. Generated during the build when empty.")

set(XMIPP4_STUB_DESTINATION "${PYTHON_INSTALL_DIR}/xmipp4/_core_binding")

# Relative to the project, so that the same value works on a runner and
# inside a build container, where the project is mounted elsewhere.
if(XMIPP4_STUBS_DIR AND NOT IS_ABSOLUTE "${XMIPP4_STUBS_DIR}")
	set(XMIPP4_STUBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${XMIPP4_STUBS_DIR}")
endif()

if(XMIPP4_STUBS_DIR)
	# Stubs are not needed at run time, so shipping none of them would go
	# unnoticed until somebody wondered why their editor stopped helping.
	file(GLOB XMIPP4_PROVIDED_STUBS "${XMIPP4_STUBS_DIR}/*.pyi")
	if(NOT XMIPP4_PROVIDED_STUBS)
		message(FATAL_ERROR
			"XMIPP4_STUBS_DIR is set to '${XMIPP4_STUBS_DIR}', which holds no "
			".pyi files."
		)
	endif()

	install(DIRECTORY "${XMIPP4_STUBS_DIR}/"
		DESTINATION "${XMIPP4_STUB_DESTINATION}"
		FILES_MATCHING PATTERN "*.pyi"
	)
else()
	install(CODE "
		execute_process(
			COMMAND \"${Python_EXECUTABLE}\"
				\"${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_stubs.py\"
				--module xmipp4._core_binding
				--search-path \"${PYTHON_INSTALL_DIR}\"
				--output \"${XMIPP4_STUB_DESTINATION}\"
			RESULT_VARIABLE XMIPP4_STUBGEN_RESULT
		)
		if(NOT XMIPP4_STUBGEN_RESULT EQUAL 0)
			message(FATAL_ERROR
				\"Could not generate the type stubs. Pass XMIPP4_STUBS_DIR \"
				\"when the extension cannot be imported by the build.\"
			)
		endif()
	")
endif()
