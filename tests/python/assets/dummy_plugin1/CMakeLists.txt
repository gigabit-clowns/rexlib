# SPDX-License-Identifier: GPL-3.0-only

add_library(dummy_plugin1 MODULE plugin.cpp)
set_target_properties(
	dummy_plugin1
	PROPERTIES
		DEFINE_SYMBOL "REXLIB_DUMMY_PLUGIN1_EXPORTING"
)
target_link_libraries(
	dummy_plugin1
	PRIVATE
		rexlib::rexlib
)
rexlib_python_enable_warnings(dummy_plugin1)

# Beside the real plugin directory rather than inside it, so these are
# only ever found by a test that asks for them by path. Deriving from
# REXLIB_PLUGINS_INSTALL_DIR keeps them there whatever the layout.
get_filename_component(
	MODULE_INSTALL_DIR
	"${REXLIB_PLUGINS_INSTALL_DIR}"
	DIRECTORY
)
if(REXLIB_PYTHON_INSTALL)
	install(
		TARGETS dummy_plugin1
		LIBRARY DESTINATION
			${REXLIB_PYTHON_PACKAGE_DIR}/${MODULE_INSTALL_DIR}/rexlib-dummy-plugins/
	)
endif()
