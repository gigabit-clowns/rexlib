# SPDX-License-Identifier: GPL-3.0-only

# Sets the warnings every target of this project is built with, so that the
# plugin and the suites are held to one standard rather than two. With
# REXLIB_CUDA_WARNINGS_AS_ERRORS they fail the build.
function(rexlib_cuda_enable_warnings TARGET)
	if(MSVC)
		target_compile_options(
			${TARGET}
			PRIVATE
				/W4
				$<$<BOOL:${REXLIB_CUDA_WARNINGS_AS_ERRORS}>:/WX>
		)
	else()
		target_compile_options(
			${TARGET}
			PRIVATE
				-Wall
				-Wextra
				-Wpedantic
				-Wundef
				$<$<BOOL:${REXLIB_CUDA_WARNINGS_AS_ERRORS}>:-Werror>
		)
	endif()
endfunction()
