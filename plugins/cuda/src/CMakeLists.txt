cmake_minimum_required(VERSION 3.18)

# Find all source files
file(
	GLOB_RECURSE
	SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

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
)
target_compile_definitions(
	${OBJECTS_NAME}
	PRIVATE
		VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
		VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
		VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
		# CMake only defines this automatically for SHARED and MODULE targets.
		XMIPP4_CUDA_EXPORTING
)
target_link_libraries(
	${OBJECTS_NAME}
	PUBLIC
		xmipp4-core
		CUDA::cudart
)

# Enable all warnings during compilation
if(MSVC)
	target_compile_options(${OBJECTS_NAME} PRIVATE /W4)
else()
	target_compile_options(${OBJECTS_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Disable min/max macros in windows
if(MSVC)
	target_compile_definitions(${OBJECTS_NAME} PUBLIC -DNOMINMAX)
endif()

# Create the plugin. It is only loaded at runtime by the core's plugin manager
# and exposes no headers, so it is a MODULE and its dependencies stay private.
set(TARGET_NAME ${PROJECT_NAME})
add_library(${TARGET_NAME} MODULE $<TARGET_OBJECTS:${OBJECTS_NAME}>)
set_target_properties(
	${TARGET_NAME}
	PROPERTIES
		CXX_STANDARD 20
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE
		${OBJECTS_NAME}
)

# Install the plugin binary. No headers and no exported config: the plugin has
# no public interface.
install(
	TARGETS ${TARGET_NAME}
	LIBRARY DESTINATION ${XMIPP4_PLUGIN_INSTALL_DIR}
	RUNTIME DESTINATION ${XMIPP4_PLUGIN_INSTALL_DIR}
)
