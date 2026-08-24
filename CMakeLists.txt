cmake_minimum_required(VERSION 3.16)

# Without this CMAKE_MSVC_DEBUG_INFORMATION_FORMAT is silently ignored.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
endif()

# Define the project
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)
project(
	rexlib
	VERSION ${PROJECT_VERSION}
	LANGUAGES C CXX
)

# Declare configuration options
option(REXLIB_BUILD_DOC "Build documentation" OFF)
option(REXLIB_ENABLE_COVERAGE "Enable code coverage" OFF)
option(REXLIB_REGISTER_TESTS_PER_BINARY "Register one test per test executable instead of one per Catch2 case" OFF)

# Add the CMake module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# Include CMake packages
include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(fetch_half)
include(fetch_spdlog)
include(fetch_boost)
include(fetch_pocketfft)
include(fetch_eigen)

# Find dependencies
find_package(Threads REQUIRED)
fetch_boost(VERSION 1.90.0)
fetch_half(VERSION 2.2.1)
fetch_spdlog(VERSION 1.17.0)
fetch_pocketfft(COMMIT c90e55b3d529f8efa40ed01a20de22405f45fc65)
fetch_eigen(VERSION 5.0.1)

if(WIN32)
	set(REXLIB_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR}/rex-plugins)
else()
	set(REXLIB_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/rex-plugins)
endif()
set(REXLIB_PLUGIN_INSTALL_DIR ${REXLIB_PLUGIN_INSTALL_DIR} CACHE PATH "Directory where the rex plugins are installed")

# Set project-wide compilation options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Add source directory
add_subdirectory(src)

# Only build tests if it is the main project
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

# Only build docs if requested
if(REXLIB_BUILD_DOC)
	add_subdirectory(doc)
endif()
