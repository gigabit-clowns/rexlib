# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches DLPack at COMMIT, that of a release, and makes it available as
# dlpack::dlpack, as its installed package does.
#
#   rexlib_python_fetch_dlpack(COMMIT <sha>)
#
# Only the header is wanted. DLPack's own CMakeLists installs it and builds a
# mock, so added as a subdirectory it would put both into the package.
# SOURCE_SUBDIR names a directory that holds no CMakeLists, so the source is
# only unpacked and the target is defined here.
function(rexlib_python_fetch_dlpack)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	FetchContent_Declare(
		dlpack
		URL "https://github.com/dmlc/dlpack/archive/${arg_COMMIT}.tar.gz"
		SOURCE_SUBDIR do-not-build
	)
	FetchContent_MakeAvailable(dlpack)

	add_library(dlpack INTERFACE)
	add_library(dlpack::dlpack ALIAS dlpack)
	target_include_directories(
		dlpack
		SYSTEM INTERFACE
			${dlpack_SOURCE_DIR}/include
	)
endfunction()
