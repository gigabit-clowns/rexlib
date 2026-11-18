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

find_program(REXLIB_RUFF NAMES ruff REQUIRED)

# ruff.toml names the Python files it checks.
add_test(
	NAME rexlib-ruff
	COMMAND ${REXLIB_RUFF} check --no-cache
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(rexlib-ruff PROPERTIES LABELS style)
