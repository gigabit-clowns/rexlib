# SPDX-License-Identifier: GPL-3.0-only

include(FetchContent)

# Fetches Eigen at COMMIT, that of a release, and makes it available as
# Eigen3::Eigen, as its installed package does.
#
#   rexlib_fetch_eigen(COMMIT <sha>)
#
# Eigen's own CMakeLists installs its headers whatever it is told, so added
# as a subdirectory it would put some 600 headers into <prefix>/include/eigen3.
# SOURCE_SUBDIR names a directory that holds no CMakeLists, so the source is
# only unpacked and the target is defined here.
function(rexlib_fetch_eigen)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "COMMIT" "")

	set(archives "https://gitlab.com/libeigen/eigen/-/archive")
	FetchContent_Declare(
		eigen
		URL "${archives}/${arg_COMMIT}/eigen-${arg_COMMIT}.tar.gz"
		SOURCE_SUBDIR do-not-build
	)
	FetchContent_MakeAvailable(eigen)

	add_library(eigen INTERFACE)
	add_library(Eigen3::Eigen ALIAS eigen)
	target_include_directories(
		eigen
		SYSTEM INTERFACE
			${eigen_SOURCE_DIR}
	)
	target_compile_features(
		eigen
		INTERFACE
			cxx_std_14
	)
endfunction()
