cmake_minimum_required(VERSION 3.16)

include(GNUInstallDirs)

set(TARGET_NAME dummy_plugin1)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp)
add_library(${TARGET_NAME} MODULE ${SOURCES})
set_target_properties(
	${TARGET_NAME}
	PROPERTIES 
		CXX_STANDARD 20
		DEFINE_SYMBOL "XMIPP4_DUMMY_PLUGIN1_EXPORTING"
)
target_link_libraries(
	${TARGET_NAME}
	PRIVATE
		xmipp4-core
)

if(WIN32)
    set(MODULE_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
else()
    set(MODULE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
endif()
install(
	TARGETS ${TARGET_NAME}
	LIBRARY DESTINATION ${MODULE_INSTALL_DIR}/xmipp4-dummy-plugins/
)
