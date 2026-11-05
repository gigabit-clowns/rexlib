# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Adds the spdlog in SOURCE_DIR to the project, where rexlib uses
# spdlog::spdlog_header_only, and spdlog::spdlog is static and position
# independent, as a plugin built in the same configure links it.
#
#   rexlib_add_spdlog(SOURCE_DIR <dir>)
#
# Added below another project, spdlog installs nothing of its own.
function(rexlib_add_spdlog)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	set(SPDLOG_BUILD_SHARED OFF)
	set(SPDLOG_BUILD_PIC ON)

	rexlib_add_dependency(
		spdlog
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK CMakeLists.txt
	)
endfunction()
