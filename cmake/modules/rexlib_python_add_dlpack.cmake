# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Makes the DLPack in SOURCE_DIR available as dlpack::dlpack, as its installed
# package does.
#
#   rexlib_python_add_dlpack(SOURCE_DIR <dir>)
#
# Only the header is wanted. DLPack's own CMakeLists installs it and builds a
# mock, so added as a subdirectory it would put both into the package. Its
# sources are left unbuilt and the target is defined here.
function(rexlib_python_add_dlpack)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		dlpack
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK include/dlpack/dlpack.h
		SOURCE_SUBDIR do-not-build
	)

	add_library(dlpack INTERFACE)
	add_library(dlpack::dlpack ALIAS dlpack)
	target_include_directories(
		dlpack
		SYSTEM INTERFACE
			"${arg_SOURCE_DIR}/include"
	)
endfunction()
