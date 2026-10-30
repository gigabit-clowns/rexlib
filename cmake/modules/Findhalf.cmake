# SPDX-License-Identifier: GPL-3.0-only

# Locates the half header, a single file that ships with no CMake package
# config anywhere, and exposes it as half::half.
#
# The header carries no version macro, so this module reports none: a
# find_package(half <version>) request cannot be honoured and is not accepted.
#
# Result variables:
#   half_FOUND        Whether the header was located
#   half_INCLUDE_DIR  The directory holding half.hpp

find_path(half_INCLUDE_DIR NAMES half.hpp PATH_SUFFIXES half)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(half REQUIRED_VARS half_INCLUDE_DIR)

if(half_FOUND AND NOT TARGET half::half)
	add_library(half::half INTERFACE IMPORTED)
	set_target_properties(
		half::half
		PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${half_INCLUDE_DIR}"
			INTERFACE_COMPILE_DEFINITIONS "HALF_ENABLE_CPP11_USER_LITERALS=0"
	)
endif()

mark_as_advanced(half_INCLUDE_DIR)
