# Build the mock plugins consumed by the tests as runtime assets.
add_subdirectory(dummy_plugin)
add_subdirectory(faulty_plugin1)
add_subdirectory(faulty_plugin2)

# Single directory where every asset is staged. Both the unitary and the
# integration test suites resolve their assets from here through an absolute
# path, so the tests no longer depend on their working directory.
set(REXLIB_TEST_ASSET_ROOT ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(REXLIB_TEST_PLUGINS_DIRECTORY ${REXLIB_TEST_ASSET_ROOT}/plugins)

# Stage the assets next to the build tree.
add_custom_target(
	rex-test-assets
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_ASSET_ROOT}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lorem_ipsum.txt ${REXLIB_TEST_ASSET_ROOT}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy-plugin> ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:faulty-plugin1> ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:faulty-plugin2> ${REXLIB_TEST_PLUGINS_DIRECTORY}
)

# Interface target that gives any test suite access to the assets: the
# assets.hpp helper header and the absolute root where they are staged.
# Consumers must additionally add a build dependency on rex-test-assets.
add_library(rex-test-assets-interface INTERFACE)
target_include_directories(
	rex-test-assets-interface
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(
	rex-test-assets-interface
	INTERFACE
		REXLIB_TEST_ASSET_ROOT="${REXLIB_TEST_ASSET_ROOT}"
)
