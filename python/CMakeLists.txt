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
	GIT_TAG v3.0.1
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
