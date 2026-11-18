# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Adds the trompeloeil in SOURCE_DIR to the project as
# trompeloeil::trompeloeil.
#
#   rexlib_add_trompeloeil(SOURCE_DIR <dir>)
#
# Added below another project, trompeloeil installs nothing of its own.
function(rexlib_add_trompeloeil)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		trompeloeil
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK CMakeLists.txt
	)
endfunction()
