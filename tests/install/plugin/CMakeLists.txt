# SPDX-License-Identifier: GPL-3.0-only

# A plugin of its own, as a third party writes one against an installed
# rexlib. It knows nothing of the tree it is in. It adds a device named
# "sample", over host memory, and a program for arange into int32 that only
# that device runs.
cmake_minimum_required(VERSION 3.18)

project(rexlib-install-plugin VERSION 0.1.0 LANGUAGES CXX)

# The public headers are C++14.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(rexlib ${REXLIB_EXPECTED_VERSION} REQUIRED)

add_library(
	rexlib-sample MODULE
	plugin_hook.cpp
	sample_arange_program.cpp
	sample_arange_program_builder.cpp
	sample_command_queue.cpp
	sample_device.cpp
	sample_device_backend.cpp
	sample_plugin.cpp
	sample_program_builder_registry.cpp
)

target_link_libraries(
	rexlib-sample
	PRIVATE
		rexlib::rexlib
)

# The entry points of REXLIB_DEFINE_PLUGIN are the only symbols exported.
set_target_properties(
	rexlib-sample
	PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)

set_property(
	SOURCE sample_device_backend.cpp sample_plugin.cpp
	APPEND PROPERTY COMPILE_DEFINITIONS
		SAMPLE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
		SAMPLE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
		SAMPLE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

rexlib_install_plugin(rexlib-sample)
