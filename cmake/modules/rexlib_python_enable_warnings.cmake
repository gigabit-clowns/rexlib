# SPDX-License-Identifier: GPL-3.0-only

# Sets the warnings every target of this project is built with, so that the
# binding and the plugins of the tests are held to one standard rather than
# two. On MSVC a warning is an error.
function(rexlib_python_enable_warnings TARGET)
	if(MSVC)
		target_compile_options(
			${TARGET}
			PRIVATE
				/W4
				/WX
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
