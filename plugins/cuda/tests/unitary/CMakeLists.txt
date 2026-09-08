cmake_minimum_required(VERSION 3.16)

# Test excecutable name
set(TESTS_NAME "${PROJECT_NAME}_unitary_tests")

# Register all source and header files
file(
	GLOB_RECURSE
	TEST_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# The plugin is a MODULE and cannot be linked against, so the tests embed
# the same objects it is built from.
add_executable(
	${TESTS_NAME}
	${TEST_SOURCES}
	$<TARGET_OBJECTS:${PROJECT_NAME}-objects>
)
set_target_properties(
	${TESTS_NAME} PROPERTIES
	CXX_STANDARD 20
)
target_link_libraries(
	${TESTS_NAME}
	PRIVATE
		${PROJECT_NAME}-deps
		Catch2::Catch2WithMain
		trompeloeil::trompeloeil
)

# The embedded objects import rexlib, whose DLL is not on the loader's
# search path, so it has to sit next to the executable that needs it.
if(WIN32)
	add_custom_command(TARGET ${TESTS_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:rexlib::rexlib> $<TARGET_FILE_DIR:${TESTS_NAME}>
		COMMENT "Copy rexlib DLL next to ${TESTS_NAME}" VERBATIM
	)
endif()

# Discover tests
catch_discover_tests(${TESTS_NAME})

# Install
install(
	TARGETS ${TESTS_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
