# SPDX-License-Identifier: GPL-3.0-only

include(rexlib_add_dependency)

# Makes the Eigen in SOURCE_DIR available as Eigen3::Eigen, as its installed
# package does.
#
#   rexlib_add_eigen(SOURCE_DIR <dir>)
#
# Eigen's own CMakeLists installs its headers whatever it is told, so added
# as a subdirectory it would put some 600 headers into <prefix>/include/eigen3.
# Its sources are left unbuilt and the target is defined here.
function(rexlib_add_eigen)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR" "")

	rexlib_add_dependency(
		eigen
		SOURCE_DIR "${arg_SOURCE_DIR}"
		CHECK Eigen/Core
		SOURCE_SUBDIR do-not-build
	)

	add_library(eigen INTERFACE)
	add_library(Eigen3::Eigen ALIAS eigen)
	target_include_directories(
		eigen
		SYSTEM INTERFACE
			"${arg_SOURCE_DIR}"
	)
	target_compile_features(
		eigen
		INTERFACE
			cxx_std_14
	)
endfunction()
