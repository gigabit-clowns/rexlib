cmake_minimum_required(VERSION 3.18)

# Find all source files
file(
	GLOB_RECURSE
	SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/*.c
		${CMAKE_CURRENT_SOURCE_DIR}/*.cu
)

# $<TARGET_OBJECTS:> carries only the objects, so what is built from them
# picks up their usage requirements here instead.
set(DEPS_NAME ${PROJECT_NAME}-deps)
add_library(${DEPS_NAME} INTERFACE)
target_link_libraries(
	${DEPS_NAME}
	INTERFACE
		rexlib::rexlib
		CUDA::cudart
		Boost::container
		Boost::intrusive
		Boost::unordered
		spdlog::spdlog
)

# The unitary tests reach the plugin's internals, which have no installed
# headers, so they include them from here.
target_include_directories(
	${DEPS_NAME}
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Disable min/max macros in windows
if(MSVC)
	target_compile_definitions(${DEPS_NAME} INTERFACE NOMINMAX)
endif()

# Compile the sources once into an object library, so that both the plugin and
# the unitary test executable can consume them. A MODULE cannot be linked
# against, which is what the tests would otherwise need.
set(OBJECTS_NAME ${PROJECT_NAME}-objects)
add_library(${OBJECTS_NAME} OBJECT ${SOURCES})
set_target_properties(
	${OBJECTS_NAME}
	PROPERTIES
		CXX_STANDARD 20
		POSITION_INDEPENDENT_CODE ON
		# Device code compiled here is linked into the module below, not
		# into each object, so a kernel may be called across translation
		# units.
		CUDA_SEPARABLE_COMPILATION ON
)
target_compile_definitions(
	${OBJECTS_NAME}
	PRIVATE
		REXLIB_CUDA_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
		REXLIB_CUDA_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
		REXLIB_CUDA_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
		# CMake only defines this automatically for SHARED and MODULE targets.
		REXLIB_CUDA_EXPORTING
)
target_link_libraries(
	${OBJECTS_NAME}
	PRIVATE
		${DEPS_NAME}
)

# Enable all warnings during compilation
if(MSVC)
	target_compile_options(${OBJECTS_NAME} PRIVATE /W4)
else()
	target_compile_options(${OBJECTS_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create the plugin. It is only loaded at runtime by the core's plugin manager
# and exposes no headers, so it is a MODULE and its dependencies stay private.
set(TARGET_NAME ${PROJECT_NAME})
add_library(${TARGET_NAME} MODULE $<TARGET_OBJECTS:${OBJECTS_NAME}>)
set_target_properties(
	${TARGET_NAME}
	PROPERTIES
		CXX_STANDARD 20
		# The device link step for the separately compiled objects above.
		# Without it their device symbols are silently left unresolved.
		CUDA_RESOLVE_DEVICE_SYMBOLS ON
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE
		${DEPS_NAME}
)

if(REXLIB_CUDA_RUNTIME_RPATH)
	set_property(
		TARGET ${TARGET_NAME}
		APPEND PROPERTY INSTALL_RPATH "${REXLIB_CUDA_RUNTIME_RPATH}"
	)
endif()

# No headers and no exported config: the plugin has no public interface.
# rexlib owns where a plugin goes and how it reaches the library, so the
# destination and the RPATH back to it come from its exported helper.
if(REXLIB_CUDA_INSTALL)
	rexlib_install_plugin(${TARGET_NAME})
endif()
