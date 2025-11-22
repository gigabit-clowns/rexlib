cmake_minimum_required(VERSION 3.16)

include(CTest)
include(FetchContent)

# Test excecutable name
set(TESTS_NAME "${PROJECT_NAME}_integration_tests")

# Register all source and header files
file(
	GLOB_RECURSE 
	TEST_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# Add the test executable
add_executable(
	${TESTS_NAME}
	${TEST_SOURCES}
)
set_target_properties(
	${TESTS_NAME} PROPERTIES 
	CXX_STANDARD 20
)
target_link_libraries(
	${TESTS_NAME} 
	PRIVATE 
		xmipp4-core
		Catch2::Catch2WithMain
)
add_dependencies(${TESTS_NAME} ${PROJECT_NAME})

# On windows targets, copy the library to the test directory
if(WIN32)
	add_custom_command(TARGET ${TESTS_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:xmipp4-core> "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Copy xmipp4-core DLL file to ${CMAKE_CURRENT_BINARY_DIR} directory" VERBATIM
	)
endif()

# Copy the plugin Shared Library
add_custom_command(TARGET ${TESTS_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_BINARY_DIR}"
)

# Discover tests
catch_discover_tests(${TESTS_NAME})

# Install
install(
	TARGETS ${TESTS_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
