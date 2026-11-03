# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy_plugin2 MODULE plugin.cpp)
set_target_properties(
	dummy_plugin2
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "$<1:${REXLIB_PYTHON_TEST_PLUGINS_DIR}>"
)
target_link_libraries(
	dummy_plugin2
	PRIVATE
		rexlib::rexlib
)
rexlib_enable_warnings(dummy_plugin2)
