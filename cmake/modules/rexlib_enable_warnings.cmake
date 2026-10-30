# SPDX-License-Identifier: GPL-3.0-only

# Turns on the warnings every target of this project is built with, so that
# the library and the suites are held to one standard rather than two.
function(rexlib_enable_warnings TARGET)
	if(MSVC)
		target_compile_options(
			${TARGET}
			PRIVATE
				/W4
				/wd4996 # getenv warning
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
