# Build the mock plugins consumed by the tests as runtime assets.
add_subdirectory(dummy_plugin)
add_subdirectory(faulty_plugin1)
add_subdirectory(faulty_plugin2)

# Single directory where every asset is staged. Both the unitary and the
# integration test suites resolve their assets from here through an absolute
# path, so the tests no longer depend on their working directory.
set(REXLIB_TEST_ASSET_ROOT ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(REXLIB_TEST_PLUGINS_DIRECTORY ${REXLIB_TEST_ASSET_ROOT}/plugins)
set(REXLIB_TEST_MRC_DIRECTORY ${REXLIB_TEST_ASSET_ROOT}/mrc)
set(REXLIB_TEST_TIFF_DIRECTORY ${REXLIB_TEST_ASSET_ROOT}/tiff)

# Where a test writes the files it creates. Beside the staged assets rather
# than inside them, so that what a test produces is never mistaken for what it
# was given, and absolute, so that a test does not depend on the directory it
# was run from.
set(REXLIB_TEST_SCRATCH_ROOT ${CMAKE_CURRENT_BINARY_DIR}/scratch)

# Stage the assets next to the build tree.
add_custom_target(
	rexlib-test-assets
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_ASSET_ROOT}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lorem_ipsum.txt ${REXLIB_TEST_ASSET_ROOT}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy-plugin> ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:faulty-plugin1> ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:faulty-plugin2> ${REXLIB_TEST_PLUGINS_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_MRC_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/mrc/EMD-3197.map ${REXLIB_TEST_MRC_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/mrc/EMD-3001.map ${REXLIB_TEST_MRC_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/image_float32_tiled.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/image_int16_big_endian.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/image_uint16_deflate_predictor.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/refused_pages_differ.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/refused_rgb.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/stack_uint8_bigtiff.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tiff/stack_uint8_lzw.tif ${REXLIB_TEST_TIFF_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${REXLIB_TEST_SCRATCH_ROOT}
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
		REXLIB_TEST_SCRATCH_ROOT="${REXLIB_TEST_SCRATCH_ROOT}"
)
