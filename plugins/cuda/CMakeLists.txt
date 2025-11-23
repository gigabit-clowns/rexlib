cmake_minimum_required(VERSION 3.18)

# Define the project
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
  xmipp4-cuda
	VERSION ${PROJECT_VERSION}
	LANGUAGES C CXX CUDA
)

# Include CMake packages
include(GNUInstallDirs)
include(CTest)

# Find xmipp4-core
if(NOT TARGET xmipp4-core)
	find_package(xmipp4-core REQUIRED)
endif()

# Find dependencies
find_package(CUDAToolkit 10 REQUIRED)

# Set project-wide compilation options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Add source directory
add_subdirectory(src)

# Only build tests if it is the main project
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
