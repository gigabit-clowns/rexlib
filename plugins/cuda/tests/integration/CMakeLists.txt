# SPDX-License-Identifier: GPL-3.0-only

add_executable(rexlib-cuda-integration-tests)
add_subdirectory(src)
target_link_libraries(
	rexlib-cuda-integration-tests
	PRIVATE
		rexlib::rexlib
		Catch2::Catch2WithMain
		# To check that what the plugin hands out really is what it says it is,
		# which only the driver can answer.
		CUDA::cudart
)
add_dependencies(rexlib-cuda-integration-tests rexlib-cuda)

# The tests load the plugin from the directory they run in, and on Windows
# the plugin and the tests import rexlib, whose DLL is not on the loader's
# search path. So both sit next to the executable.
add_custom_command(
	TARGET rexlib-cuda-integration-tests
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:rexlib-cuda>
		$<TARGET_FILE_DIR:rexlib-cuda-integration-tests>
	VERBATIM
)
if(WIN32)
	add_custom_command(
		TARGET rexlib-cuda-integration-tests
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:rexlib::rexlib>
			$<TARGET_FILE_DIR:rexlib-cuda-integration-tests>
		VERBATIM
	)
endif()

rexlib_cuda_enable_warnings(rexlib-cuda-integration-tests)
rexlib_cuda_register_tests(rexlib-cuda-integration-tests integration)
