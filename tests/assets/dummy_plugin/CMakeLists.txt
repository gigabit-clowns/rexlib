# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy-plugin MODULE dummy_plugin.cpp)
target_link_libraries(
	dummy-plugin
	PRIVATE
		rexlib
)
rexlib_enable_warnings(dummy-plugin)
