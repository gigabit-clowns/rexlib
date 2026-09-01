cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)

set(TARGET_NAME dummy_plugin1)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp)
add_library(${TARGET_NAME} MODULE ${SOURCES})
set_target_properties(
	${TARGET_NAME}
	PROPERTIES 
		CXX_STANDARD 20
		DEFINE_SYMBOL "REXLIB_DUMMY_PLUGIN1_EXPORTING"
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE
		rexlib::rexlib
)

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
		TARGETS ${TARGET_NAME}
		LIBRARY DESTINATION
			${REXLIB_PYTHON_PACKAGE_DIR}/${MODULE_INSTALL_DIR}/rexlib-dummy-plugins/
	)
endif()
