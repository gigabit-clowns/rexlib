# SPDX-License-Identifier: GPL-3.0-only

# Sets the warnings every target of this project is built with, so that the
# plugin and the suites are held to one standard rather than two.
function(rexlib_cuda_enable_warnings TARGET)
	if(MSVC)
		target_compile_options(
			${TARGET}
			PRIVATE
				/W4
		)
	else()
		target_compile_options(
			${TARGET}
			PRIVATE
				-Wall
				-Wextra
				-Wpedantic
		)
	endif()
endfunction()
