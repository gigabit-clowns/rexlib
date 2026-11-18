# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-core-obj
	PRIVATE
		dynamic_library.cpp
		environment.cpp
		host.cpp
		utf8_path.cpp
)

if(WIN32)
	target_sources(
		rexlib-core-obj
		PRIVATE
			dynamic_library_handle_windows.cpp
			utf16_windows.cpp
	)
else()
	target_sources(
		rexlib-core-obj
		PRIVATE
			dynamic_library_handle_posix.cpp
	)
endif()
