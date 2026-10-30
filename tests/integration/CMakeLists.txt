# SPDX-License-Identifier: GPL-3.0-only

add_executable(rexlib-integration-tests)
add_subdirectory(src)
target_link_libraries(
	rexlib-integration-tests
	PRIVATE
		rexlib
		rexlib-test-assets-interface
		Catch2::Catch2WithMain
)
add_dependencies(
	rexlib-integration-tests
	rexlib-test-assets
)

rexlib_enable_warnings(rexlib-integration-tests)

rexlib_register_tests(rexlib-integration-tests integration)
