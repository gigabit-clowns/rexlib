# SPDX-License-Identifier: GPL-3.0-only

# $<TARGET_OBJECTS:> carries only the objects, so what is built from them
# picks up their usage requirements here instead.
add_library(rexlib-cuda-deps INTERFACE)
target_link_libraries(
	rexlib-cuda-deps
	INTERFACE
		rexlib::rexlib
		CUDA::cudart
		Boost::headers # Container, Intrusive and Unordered
		spdlog::spdlog
)

# The unitary tests reach the plugin's internals, which have no installed
# headers, so they include them from here.
target_include_directories(
	rexlib-cuda-deps
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Without it windows.h defines min and max as macros.
if(MSVC)
	target_compile_definitions(
		rexlib-cuda-deps
		INTERFACE
			NOMINMAX
	)
endif()

# Compile the sources once into an object library, so that both the plugin and
# the unitary test executable can consume them. A MODULE cannot be linked
# against, which is what the tests would otherwise need.
add_library(rexlib-cuda-objects OBJECT)
target_sources(
	rexlib-cuda-objects
	PRIVATE
		cuda_plugin.cpp
		plugin_hook.cpp
)
add_subdirectory(hardware)

set_target_properties(
	rexlib-cuda-objects
	PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		# Device code compiled here is linked into the module below, not
		# into each object, so a kernel may be called across translation
		# units.
		CUDA_SEPARABLE_COMPILATION ON
)
target_compile_definitions(
	rexlib-cuda-objects
	PRIVATE
		REXLIB_CUDA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
		REXLIB_CUDA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
		REXLIB_CUDA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
		# CMake only defines this automatically for SHARED and MODULE targets.
		REXLIB_CUDA_EXPORTING
)
target_link_libraries(
	rexlib-cuda-objects
	PRIVATE
		rexlib-cuda-deps
)
rexlib_cuda_enable_warnings(rexlib-cuda-objects)

# Create the plugin. It is only loaded at runtime by the core's plugin manager
# and exposes no headers, so it is a MODULE and its dependencies stay private.
add_library(rexlib-cuda MODULE $<TARGET_OBJECTS:rexlib-cuda-objects>)
set_target_properties(
	rexlib-cuda
	PROPERTIES
		# The device link step for the separately compiled objects above.
		# Without it their device symbols are silently left unresolved.
		CUDA_RESOLVE_DEVICE_SYMBOLS ON
)
target_link_libraries(
	rexlib-cuda
	PRIVATE
		rexlib-cuda-deps
)

if(REXLIB_CUDA_RUNTIME_RPATH)
	set_property(
		TARGET rexlib-cuda
		APPEND PROPERTY INSTALL_RPATH "${REXLIB_CUDA_RUNTIME_RPATH}"
	)
endif()

# No headers and no exported config: the plugin has no public interface.
# rexlib owns where a plugin goes and how it reaches the library, so the
# destination and the RPATH back to it come from its exported helper.
if(REXLIB_CUDA_INSTALL)
	rexlib_install_plugin(rexlib-cuda)
endif()
