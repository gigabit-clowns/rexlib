# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy_plugin1 MODULE plugin.cpp)
set_target_properties(
	dummy_plugin1
	PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "$<1:${REXLIB_PYTHON_TEST_PLUGINS_DIR}>"
)
target_link_libraries(
	dummy_plugin1
	PRIVATE
		rexlib::rexlib
)
rexlib_enable_warnings(dummy_plugin1)
