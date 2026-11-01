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
	rexlib-cuda
	VERSION ${PROJECT_VERSION}
	DESCRIPTION "CUDA hardware backend for rexlib"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CTest)
include(GNUInstallDirs)
include(CMakeDependentOption)
include(rexlib_cuda_architectures)
include(rexlib_cuda_enable_warnings)

option(REXLIB_CUDA_INSTALL "Generate the plugin's install rules" ON)
option(
	REXLIB_CUDA_REGISTER_TESTS_PER_BINARY
	"Register one test per test executable instead of one per Catch2 case"
	OFF
)
# Only while BUILD_TESTING, the switch of CTest, is on: tests built without
# it could not be registered with CTest. Its own option lets a project that
# embeds the plugin drop its tests without dropping its own.
cmake_dependent_option(
	REXLIB_CUDA_BUILD_TESTING "Build the plugin's tests" ON
	"BUILD_TESTING" OFF
)

set(
	REXLIB_CUDA_REXLIB_VERSION 0.1
	CACHE STRING "Version of rexlib this plugin requires"
)
set(
	REXLIB_CUDA_RUNTIME_RPATH ""
	CACHE STRING "RPATH entry reaching the CUDA runtime"
)

# Every dependency but rexlib is private to the plugin, which has no public
# interface, so building one here or finding an installed one changes nothing
# for whoever loads it. Turning the global switch off turns every one of the
# others off, on every configure.
option(
	REXLIB_CUDA_BUILD_DEPENDENCIES
	"Build the dependencies with the plugin, rather than finding installed \
ones"
	ON
)
cmake_dependent_option(
	REXLIB_CUDA_BUILD_BOOST "Fetch Boost and build it with the plugin" ON
	"REXLIB_CUDA_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_CUDA_BUILD_SPDLOG "Fetch spdlog and build it with the plugin" ON
	"REXLIB_CUDA_BUILD_DEPENDENCIES" OFF
)

# The plugin is C++20. The standard is required rather than preferred, so
# that nothing newer gets in unnoticed.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CUDA is enabled here rather than in project() because the architectures have
# to be settled first, and the compiler is what reports them. enable_language()
# would otherwise pick a default and leave nothing to settle.
find_package(CUDAToolkit 11 REQUIRED)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
	rexlib_cuda_all_major_architectures(
		CMAKE_CUDA_ARCHITECTURES "${CUDAToolkit_NVCC_EXECUTABLE}"
	)
endif()

enable_language(CUDA)

if(REXLIB_CUDA_BUILD_BOOST)
	include(rexlib_cuda_fetch_boost)
	rexlib_cuda_fetch_boost(
		TAG boost-1.92.0
		SHA256 9bed76128d4e46755dbe818487788c6fceb6f72b378f4daa49b7e1e600d9088d
	)
else()
	# unordered_flat_map arrived in Boost 1.81.
	find_package(Boost 1.81 REQUIRED CONFIG)
endif()

if(REXLIB_CUDA_BUILD_SPDLOG)
	include(rexlib_cuda_fetch_spdlog)
	rexlib_cuda_fetch_spdlog(
		COMMIT 79524ddd08a4ec981b7fea76afd08ee05f83755d
	) # v1.17.0
else()
	find_package(spdlog 1.4 REQUIRED CONFIG)
endif()

if(NOT TARGET rexlib::rexlib)
	find_package(rexlib ${REXLIB_CUDA_REXLIB_VERSION} REQUIRED)
endif()

add_subdirectory(src)

if(REXLIB_CUDA_BUILD_TESTING)
	add_subdirectory(tests)
endif()
