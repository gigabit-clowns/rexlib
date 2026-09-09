cmake_minimum_required(VERSION 3.18)

# Without this CMAKE_MSVC_DEBUG_INFORMATION_FORMAT is silently ignored.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
endif()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
	rexlib-cuda
	VERSION ${PROJECT_VERSION}
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CTest)
include(GNUInstallDirs)
include(fetch_boost)
include(fetch_spdlog)
include(cuda_architectures)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

option(REXLIB_CUDA_BUILD_TESTING "Build this project's tests" ${BUILD_TESTING})

option(REXLIB_CUDA_INSTALL "Generate install rules" ON)

set(REXLIB_CUDA_REXLIB_VERSION 0.1 CACHE STRING
	"Version of rexlib this plugin requires")

set(REXLIB_CUDA_RUNTIME_RPATH "" CACHE STRING "RPATH entry reaching the CUDA runtime")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# CUDA is enabled here rather than in project() because the architectures have
# to be settled first, and the compiler is what reports them. enable_language()
# would otherwise pick a default and leave nothing to settle.
find_package(CUDAToolkit 11 REQUIRED)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
	cuda_all_major_architectures(
		CMAKE_CUDA_ARCHITECTURES "${CUDAToolkit_NVCC_EXECUTABLE}"
	)
endif()

enable_language(CUDA)

fetch_boost(VERSION 1.92.0)
fetch_spdlog(VERSION 1.17.0)

if(NOT TARGET rexlib::rexlib)
	find_package(rexlib ${REXLIB_CUDA_REXLIB_VERSION} REQUIRED)
endif()

# ---------------------------------------------------------------------------
# The plugin
# ---------------------------------------------------------------------------

add_subdirectory(src)

if(REXLIB_CUDA_BUILD_TESTING)
	add_subdirectory(tests)
endif()
