# Define one object library per component. Properties that CMake only
# applies automatically to SHARED targets (export define, PIC, hidden
# visibility) must be set explicitly on each object library.
#
# A component links only the dependencies its own sources reach, so that the
# boundary between the subsystems is one CMake keeps rather than one a reader
# has to. They are PUBLIC on these targets, which are internal and never
# exported: the unitary suite compiles against the same headers, so it has to
# see the same include directories.
#
# Each is created empty and filled in by the CMakeLists of the directories it
# spans, one per directory, through target_sources.
function(rexlib_add_component NAME)
	add_library(${NAME} OBJECT)
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
			${PROJECT_SOURCE_DIR}/src
	)
	target_compile_definitions(
		${NAME}
		PRIVATE
			REXLIB_EXPORTING
			REXLIB_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
			REXLIB_VERSION_MINOR=${PROJECT_VERSION_MINOR}
			REXLIB_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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

	rexlib_enable_warnings(${NAME})

	# Add coverage flags if requested
	if(REXLIB_ENABLE_COVERAGE)
		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			target_compile_options(${NAME} PRIVATE --coverage -pthread -O0 -g)
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			target_compile_options(${NAME} PRIVATE --coverage -O0 -g)
		elseif(MSVC)
			target_compile_options(${NAME} PRIVATE /Z7 /Od)
		endif()
	endif()

endfunction()

# What each component links is what its sources reach, headers of the other
# components included. spdlog and half are the two that travel: core/logger.hpp
# includes one and core/numerical/float16_conversions.hpp the other, so every
# component that includes either of those headers needs them too.
rexlib_add_component(rexlib-core-obj)
target_link_libraries(
	rexlib-core-obj
	PUBLIC
		Threads::Threads
		spdlog::spdlog_header_only
		half::half
		Boost::filesystem # Eventually replace by std::filesystem (C++17)
		Boost::container # Eventually replace in C++23
		Boost::container_hash
		${CMAKE_DL_LIBS} # Loading the plugins
)
if(WIN32)
	target_link_libraries(rexlib-core-obj PUBLIC ws2_32) # gethostname
endif()

# Operation declarations, and nothing outside the standard library and this
# project's own headers.
rexlib_add_component(rexlib-ops-obj)

rexlib_add_component(rexlib-functional-obj)
target_link_libraries(
	rexlib-functional-obj
	PUBLIC
		Threads::Threads
		spdlog::spdlog_header_only
)

rexlib_add_component(rexlib-cpu-obj)
target_link_libraries(
	rexlib-cpu-obj
	PUBLIC
		Threads::Threads
		spdlog::spdlog_header_only
		half::half
		pocketfft::pocketfft
		Eigen3::Eigen
)

rexlib_add_component(rexlib-em-obj)
target_link_libraries(
	rexlib-em-obj
	PUBLIC
		Threads::Threads
		spdlog::spdlog_header_only
		half::half
		Boost::filesystem
		Boost::container
		Boost::container_hash
		Boost::interprocess
)

# The sources, one CMakeLists per directory. They come after every component
# above, since each names the target it adds itself to.
add_subdirectory(core)
add_subdirectory(ops)
add_subdirectory(functional)
add_subdirectory(backends)
add_subdirectory(em)

# Assemble the single shared library from the component objects.
#
# The components are linked rather than spliced in with $<TARGET_OBJECTS:>,
# which carries only the object files: linking an object library brings its
# usage requirements and its own link dependencies along with them, so the
# private dependencies above reach this link without being named twice.
#
# SHARED unconditionally, BUILD_SHARED_LIBS deliberately not honoured. A
# plugin is a module that links this library and is loaded into a process that
# already has it, so there has to be one copy of its state, not one per
# plugin. Note that a static rexlib would also break install(EXPORT), which
# only tolerates the unexported targets below because a shared library keeps
# its private dependencies out of its interface.
add_library(rexlib SHARED)
target_link_libraries(
	rexlib
	PRIVATE
		rexlib-core-obj
		rexlib-ops-obj
		rexlib-functional-obj
		rexlib-cpu-obj
		rexlib-em-obj
)
set_target_properties(
	rexlib
	PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
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

# The one dependency a consumer of the installed package inherits, which is
# why the package config finds it again. The rest arrive privately with the
# components above.
target_link_libraries(rexlib PUBLIC Threads::Threads)

# Create the alias
add_library(rexlib::rexlib ALIAS rexlib)

if(NOT REXLIB_INSTALL)
	return()
endif()

# Install library's binary files and headers.
install(
	TARGETS rexlib
	EXPORT rexlib-targets
	RUNTIME
		DESTINATION ${CMAKE_INSTALL_BINDIR}
		COMPONENT rexlib_runtime
	LIBRARY
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT rexlib_runtime
		NAMELINK_COMPONENT rexlib_namelink
	ARCHIVE
		DESTINATION ${CMAKE_INSTALL_LIBDIR}
		COMPONENT rexlib_development
)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT rexlib_development
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
	COMPONENT rexlib_development
)
install(
	FILES
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/rexlib-config-version.cmake
	${PROJECT_SOURCE_DIR}/cmake/config/rexlib-plugin.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rexlib
	COMPONENT rexlib_development
)
