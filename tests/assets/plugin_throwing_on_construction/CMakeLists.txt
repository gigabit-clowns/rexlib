# SPDX-License-Identifier: GPL-3.0-only

add_library(
	plugin-throwing-on-construction
	MODULE
		plugin_throwing_on_construction.cpp
)
target_link_libraries(
	plugin-throwing-on-construction
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-throwing-on-construction)
