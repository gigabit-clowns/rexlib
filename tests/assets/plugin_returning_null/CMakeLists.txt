# SPDX-License-Identifier: GPL-3.0-only

add_library(plugin-returning-null MODULE plugin_returning_null.cpp)
target_link_libraries(
	plugin-returning-null
	PRIVATE
		rexlib
)
rexlib_enable_warnings(plugin-returning-null)
