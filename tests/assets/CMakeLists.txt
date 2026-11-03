# SPDX-License-Identifier: GPL-3.0-only

# Build the mock plugins consumed by the tests as runtime assets.
add_subdirectory(dummy_plugin)
add_subdirectory(plugin_for_another_abi)
add_subdirectory(plugin_for_another_minor)
add_subdirectory(plugin_returning_null)
add_subdirectory(plugin_without_compatibility)
add_subdirectory(plugin_without_entry_point)
set(
	REXLIB_TEST_PLUGINS
	dummy-plugin
	plugin-for-another-abi
	plugin-for-another-minor
	plugin-returning-null
	plugin-without-compatibility
	plugin-without-entry-point
)

# Only the linkers of Unix leave an unresolved symbol to the loader.
if(UNIX)
	add_subdirectory(plugin_with_missing_symbol)
	list(APPEND REXLIB_TEST_PLUGINS plugin-with-missing-symbol)
endif()

# Single directory where every asset is staged. Both the unitary and the
# integration test suites resolve their assets from here through an absolute
# path, so the tests no longer depend on their working directory.
set(REXLIB_TEST_ASSET_ROOT ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(REXLIB_TEST_PLUGINS_DIRECTORY ${REXLIB_TEST_ASSET_ROOT}/plugins)

set(REXLIB_COPY_TEST_PLUGINS)
foreach(PLUGIN ${REXLIB_TEST_PLUGINS})
	list(
		APPEND REXLIB_COPY_TEST_PLUGINS
		COMMAND
			${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:${PLUGIN}>
			${REXLIB_TEST_PLUGINS_DIRECTORY}
	)
endforeach()

# Stage the assets next to the build tree.
add_custom_target(
	rexlib-test-assets
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_ASSET_ROOT}
	COMMAND
		${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/lorem_ipsum.txt
		${REXLIB_TEST_ASSET_ROOT}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_PLUGINS_DIRECTORY}
	${REXLIB_COPY_TEST_PLUGINS}
	COMMAND
		${CMAKE_COMMAND}
		-DPLUGIN=$<TARGET_FILE:dummy-plugin>
		-DROOT=${REXLIB_TEST_ASSET_ROOT}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/copy_to_non_ascii_directory.cmake
)

# Interface target that gives any test suite access to the assets: the
# assets.hpp helper header and the absolute root where they are staged.
# Consumers must additionally add a build dependency on rexlib-test-assets.
add_library(rexlib-test-assets-interface INTERFACE)
target_include_directories(
	rexlib-test-assets-interface
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(
	rexlib-test-assets-interface
	INTERFACE
		REXLIB_TEST_ASSET_ROOT="${REXLIB_TEST_ASSET_ROOT}"
)
