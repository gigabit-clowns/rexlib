# SPDX-License-Identifier: GPL-3.0-only

add_library(plugin-without-entry-point MODULE plugin_without_entry_point.cpp)
target_link_libraries(
	plugin-without-entry-point
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-without-entry-point)
