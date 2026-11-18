# SPDX-License-Identifier: GPL-3.0-only

add_library(plugin-for-another-abi MODULE plugin_for_another_abi.cpp)
target_link_libraries(
	plugin-for-another-abi
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-for-another-abi)
