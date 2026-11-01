# SPDX-License-Identifier: GPL-3.0-only

# The plugin is a MODULE and cannot be linked against, so the tests embed
# the same objects it is built from.
add_executable(
	rexlib-cuda-unitary-tests
	$<TARGET_OBJECTS:rexlib-cuda-objects>
)
add_subdirectory(src)
target_link_libraries(
	rexlib-cuda-unitary-tests
	PRIVATE
		rexlib-cuda-deps
		Catch2::Catch2WithMain
		trompeloeil::trompeloeil
)

# The embedded objects import rexlib, whose DLL is not on the loader's
# search path, so it has to sit next to the executable that needs it.
if(WIN32)
	add_custom_command(
		TARGET rexlib-cuda-unitary-tests
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:rexlib::rexlib>
			$<TARGET_FILE_DIR:rexlib-cuda-unitary-tests>
		COMMENT "Copy rexlib DLL next to rexlib-cuda-unitary-tests"
		VERBATIM
	)
endif()

rexlib_cuda_enable_warnings(rexlib-cuda-unitary-tests)
rexlib_cuda_register_tests(rexlib-cuda-unitary-tests unitary)
