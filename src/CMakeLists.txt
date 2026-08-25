cmake_minimum_required(VERSION 3.16)

# Usage requirements of the private dependencies, shared by every component
# object library and by the final shared library.
add_library(rexlib-private-deps INTERFACE)
target_link_libraries(
	rexlib-private-deps
	INTERFACE
		Threads::Threads
		spdlog::spdlog_header_only
		Boost::filesystem # Eventually replace by std::filesystem (C++17)
		Boost::container # Eventually replace in C++23
		Boost::unordered # Eventually replace in C++23
		Boost::intrusive
		half
		pocketfft
		Eigen3::Eigen
		${CMAKE_DL_LIBS}
)
if(WIN32)
	target_link_libraries(
		rexlib-private-deps
		INTERFACE
			ws2_32 # Winsock
	)
endif()

# Define one object library per component. Properties that CMake only
# applies automatically to SHARED targets (export define, PIC, hidden
# visibility) must be set explicitly on each object library.
function(rexlib_add_component NAME DIR)
	file(
		GLOB_RECURSE
		SOURCES
			${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/*.c
	)
	add_library(${NAME} OBJECT ${SOURCES})
	set_target_properties(
		${NAME}
		PROPERTIES
			POSITION_INDEPENDENT_CODE ON
			C_VISIBILITY_PRESET hidden
			CXX_VISIBILITY_PRESET hidden
	)
	target_include_directories(
		${NAME}
		PUBLIC
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}
	)
	target_compile_definitions(
		${NAME}
		PRIVATE
			REXLIB_EXPORTING
			REXLIB_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
			REXLIB_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
			REXLIB_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
	)

	# On Windows disable min and max macros
	if(MSVC)
		target_compile_definitions(
			${NAME}
			PRIVATE
				NOMINMAX
		)
	endif()

	# On Windows enable big objects
	if(MSVC)
		target_compile_options(
			${NAME}
			PRIVATE
				/bigobj
		)
	elseif(MINGW)
		target_compile_options(
			${NAME}
			PRIVATE
				-Wa,-mbig-obj
		)
	endif()

	# Enable warnings
	if(MSVC)
		target_compile_options(
			${NAME}
			PRIVATE
				/W4
				/wd4996 # getenv warning
		)
	else()
		target_compile_options(
			${NAME}
			PRIVATE
				-Wall
				-Wextra
				-Wpedantic
		)
	endif()

	# Add coverage flags if requested
	if(REXLIB_ENABLE_COVERAGE)
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
			target_compile_options(${NAME} PRIVATE --coverage -O0 -g)
		elseif(MSVC)
			target_compile_options(${NAME} PRIVATE /Z7 /Od)
		endif()
	endif()

	target_link_libraries(
		${NAME}
		PRIVATE
			rexlib-private-deps
	)
endfunction()

rexlib_add_component(rexlib-core-obj core)
rexlib_add_component(rexlib-ops-obj ops)
rexlib_add_component(rexlib-functional-obj functional)
rexlib_add_component(rexlib-cpu-obj backends/cpu)
# NOTE: register src/em as rexlib-em-obj once it has sources (an empty
# GLOB would make add_library fail).

# Assemble the single shared library from the component objects
add_library(
	rexlib
	SHARED
		$<TARGET_OBJECTS:rexlib-core-obj>
		$<TARGET_OBJECTS:rexlib-ops-obj>
		$<TARGET_OBJECTS:rexlib-functional-obj>
		$<TARGET_OBJECTS:rexlib-cpu-obj>
)
set_target_properties(
	rexlib
	PROPERTIES
		SOVERSION ${PROJECT_VERSION}
)
target_include_directories(
	rexlib
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
	target_compile_definitions(
		rexlib
		PUBLIC
			NOMINMAX
	)
endif()

# Add coverage flags if requested
if(REXLIB_ENABLE_COVERAGE)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_link_options(rexlib PUBLIC --coverage)
	elseif(MSVC)
		target_link_options(rexlib PUBLIC /DEBUG)
	endif()
endif()

# Link dependencies
target_link_libraries(
	rexlib
	PUBLIC
		Threads::Threads
	PRIVATE
		rexlib-private-deps
)

# Create the alias
add_library(rexlib::rexlib ALIAS rexlib)

# Install library's binary files and headers
install(
	TARGETS rexlib
	EXPORT rexlib-targets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "*.inl"
)

# Create and install CMake config files
configure_package_config_file(
	${PROJECT_SOURCE_DIR}/cmake/config/config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rexlib
)
write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config-version.cmake
	COMPATIBILITY SameMinorVersion
)
install(
	EXPORT rexlib-targets
	FILE rexlib-targets.cmake
	NAMESPACE rexlib::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rexlib
)
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config-version.cmake
	${PROJECT_SOURCE_DIR}/cmake/config/rexlib-plugin.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rexlib
)
