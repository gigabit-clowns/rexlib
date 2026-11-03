# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches spdlog at COMMIT, that of a release, and builds it with the
# project, static and position independent, as spdlog::spdlog.
#
#   rexlib_cuda_fetch_spdlog(COMMIT <sha>)
#
# The plugin is a module, so what it links has to be position independent.
# Added below another project, spdlog installs nothing of its own.
function(rexlib_cuda_fetch_spdlog)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	set(SPDLOG_BUILD_SHARED OFF)
	set(SPDLOG_BUILD_PIC ON)

	FetchContent_Declare(
		spdlog
		URL "https://github.com/gabime/spdlog/archive/${arg_COMMIT}.tar.gz"
	)
	FetchContent_MakeAvailable(spdlog)
endfunction()
