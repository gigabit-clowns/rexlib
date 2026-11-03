# SPDX-License-Identifier: GPL-3.0-only

add_library(plugin-for-another-minor MODULE plugin_for_another_minor.cpp)
target_link_libraries(
	plugin-for-another-minor
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-for-another-minor)
