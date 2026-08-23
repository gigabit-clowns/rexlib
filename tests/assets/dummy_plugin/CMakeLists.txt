cmake_minimum_required(VERSION 3.16)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/dummy_plugin.cpp)
add_library(dummy-plugin MODULE ${SOURCES})
set_target_properties(
	dummy-plugin
	PROPERTIES 
		DEFINE_SYMBOL "REX_DUMMY_PLUGIN_EXPORTING"
)
target_link_libraries(
	dummy-plugin
	PRIVATE
		rexlib
)
