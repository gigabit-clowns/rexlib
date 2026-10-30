# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches trompeloeil at COMMIT, that of a release, and adds it to the
# project as trompeloeil::trompeloeil.
#
#   rexlib_fetch_trompeloeil(COMMIT <sha>)
#
# Added below another project, trompeloeil installs nothing of its own.
function(rexlib_fetch_trompeloeil)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	set(archives "https://github.com/rollbear/trompeloeil/archive")
	FetchContent_Declare(
		trompeloeil
		URL "${archives}/${arg_COMMIT}.tar.gz"
	)
	FetchContent_MakeAvailable(trompeloeil)
endfunction()
