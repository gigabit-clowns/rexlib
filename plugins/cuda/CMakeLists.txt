# SPDX-License-Identifier: GPL-3.0-only

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
option(
	REXLIB_CUDA_WARNINGS_AS_ERRORS
	"Fail the build of the plugin's own targets on a compiler warning"
	OFF
)


# Every dependency but rexlib is private to the plugin, which has no public
# interface, so building one here or finding an installed one changes nothing
# for whoever loads it. Turning the global switch on turns every one of the
# others on, on every configure.
option(
	REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES
	"Find every dependency installed, rather than fetching it and building it \
with the plugin"
	OFF
)
cmake_dependent_option(
	REXLIB_CUDA_USE_SYSTEM_BOOST
	"Find an installed Boost rather than fetching it"
	OFF
	"NOT REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_CUDA_USE_SYSTEM_SPDLOG
	"Find an installed spdlog rather than fetching it"
	OFF
	"NOT REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES"
	ON
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

if(REXLIB_CUDA_USE_SYSTEM_BOOST)
	# unordered_flat_map arrived in Boost 1.81.
	find_package(Boost 1.81 REQUIRED CONFIG)
else()
	include(rexlib_cuda_add_boost)
	rexlib_cuda_add_boost(
		SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/boost
		LIBRARIES container intrusive unordered
	)
endif()

if(REXLIB_CUDA_USE_SYSTEM_SPDLOG)
	find_package(spdlog 1.4 REQUIRED CONFIG)
else()
	include(rexlib_cuda_add_spdlog)
	rexlib_cuda_add_spdlog(SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/spdlog)
endif()

# The plugin is built against an installed rexlib, as any third-party
# plugin is, of the minor version it shares with the library; inside rexlib's
# own build, against the library being built.
if(NOT TARGET rexlib::rexlib)
	find_package(
		rexlib ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} REQUIRED
	)
endif()

add_subdirectory(src)

if(REXLIB_CUDA_BUILD_TESTING)
	add_subdirectory(tests)
endif()
