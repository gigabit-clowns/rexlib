cmake_minimum_required(VERSION 3.16)

# Register all source and header files
file(
	GLOB_RECURSE
	TEST_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# Add the test executable
add_executable(rexlib-integration-tests ${TEST_SOURCES})
target_link_libraries(
	rexlib-integration-tests
	PRIVATE
		rexlib
		xmipp4-test-assets-interface
		Catch2::Catch2WithMain
)
add_dependencies(
	rexlib-integration-tests
	xmipp4-test-assets
)

# On windows targets, copy the library to the test directory
if(WIN32)
	add_custom_command(TARGET rexlib-integration-tests
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rexlib> $<TARGET_FILE_DIR:rexlib-integration-tests>
	)
endif()

# Register tests
xmipp4_register_tests(rexlib-integration-tests)
