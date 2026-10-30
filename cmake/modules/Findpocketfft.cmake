# SPDX-License-Identifier: GPL-3.0-only

# Locates the pocketfft header, a single file that ships with no CMake of any
# kind, and exposes it as pocketfft::pocketfft.
#
# pocketfft makes no releases and its header carries no version macro, so this
# module reports none: a find_package(pocketfft <version>) request cannot be
# honoured and is not accepted.
#
# Result variables:
#   pocketfft_FOUND        Whether the header was located
#   pocketfft_INCLUDE_DIR  The directory holding pocketfft_hdronly.h

find_path(
	pocketfft_INCLUDE_DIR
	NAMES pocketfft_hdronly.h
	PATH_SUFFIXES pocketfft
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	pocketfft
	REQUIRED_VARS pocketfft_INCLUDE_DIR
)

if(pocketfft_FOUND AND NOT TARGET pocketfft::pocketfft)
	add_library(pocketfft::pocketfft INTERFACE IMPORTED)
	set_target_properties(
		pocketfft::pocketfft
		PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${pocketfft_INCLUDE_DIR}"
	)
endif()

mark_as_advanced(pocketfft_INCLUDE_DIR)
