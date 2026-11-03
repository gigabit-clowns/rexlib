# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_access_flags.cpp
		test_dynamic_library.cpp
		test_environment.cpp
		test_utf8_path.cpp
)

if(WIN32)
	target_sources(
		rexlib-unitary-tests
		PRIVATE
			test_utf16_windows.cpp
	)
endif()
