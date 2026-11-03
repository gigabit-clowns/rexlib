# SPDX-License-Identifier: GPL-3.0-only

find_program(
	REXLIB_EDITORCONFIG_CHECKER
	NAMES editorconfig-checker ec
	REQUIRED
)

# With no files named, editorconfig-checker checks every file git tracks.
add_test(
	NAME rexlib-style
	COMMAND ${REXLIB_EDITORCONFIG_CHECKER} -no-color
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(rexlib-style PROPERTIES LABELS style)
