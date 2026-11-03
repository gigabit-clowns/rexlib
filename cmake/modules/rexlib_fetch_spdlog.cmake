# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches spdlog at COMMIT, that of a release, and adds it to the project,
# where rexlib uses spdlog::spdlog_header_only.
#
#   rexlib_fetch_spdlog(COMMIT <sha>)
#
# Added below another project, spdlog installs nothing of its own.
function(rexlib_fetch_spdlog)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	# From CMake 3.25 its headers count as system headers, whose
	# warnings are not rexlib's.
	set(system "")
	if(NOT CMAKE_VERSION VERSION_LESS 3.25)
		set(system SYSTEM)
	endif()
	FetchContent_Declare(
		spdlog
		${system}
		URL "https://github.com/gabime/spdlog/archive/${arg_COMMIT}.tar.gz"
	)
	FetchContent_MakeAvailable(spdlog)
endfunction()
