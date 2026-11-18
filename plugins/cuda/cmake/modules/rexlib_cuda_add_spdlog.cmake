# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_cuda_add_dependency)

# Builds the spdlog in SOURCE_DIR with the project, static and position
# independent, as spdlog::spdlog.
#
#   rexlib_cuda_add_spdlog(SOURCE_DIR <dir>)
#
# The plugin is a module, so what it links has to be position independent.
# Added below another project, spdlog installs nothing of its own.
function(rexlib_cuda_add_spdlog)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	set(SPDLOG_BUILD_SHARED OFF)
	set(SPDLOG_BUILD_PIC ON)

	rexlib_cuda_add_dependency(
		spdlog
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK CMakeLists.txt
	)
endfunction()
