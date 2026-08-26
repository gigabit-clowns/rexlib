cmake_minimum_required(VERSION 3.18)

include(FetchContent)

# Eigen is a private dependency: it is linked PRIVATE and appears in no
# public header, so none of it belongs in an install of this project.
#
# Its CMakeLists installs its headers unconditionally - there is no option
# to turn that off - so adding it as a subdirectory would put (and overwrite) 
# ~600 headers into <prefix>/include/eigen3. Pointing SOURCE_SUBDIR at a 
# directory that holds no CMakeLists.txt makes FetchContent populate the 
# source without calling add_subdirectory(), which leaves its install rules 
# out of this build entirely.
function(fetch_eigen)
	set(options)
	set(oneValueArgs VERSION)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	cmake_policy(SET CMP0135 NEW) # To avoid warnings
	FetchContent_Declare(
		eigen
		URL https://gitlab.com/libeigen/eigen/-/archive/${arg_VERSION}/eigen-${arg_VERSION}.tar.gz
		SOURCE_SUBDIR do-not-build
	)

	FetchContent_MakeAvailable(eigen)

	if(NOT TARGET Eigen3::Eigen)
		add_library(eigen INTERFACE)
		add_library(Eigen3::Eigen ALIAS eigen)
		target_include_directories(eigen SYSTEM INTERFACE "${eigen_SOURCE_DIR}")
		target_compile_features(eigen INTERFACE cxx_std_14)
	endif()
endfunction()
