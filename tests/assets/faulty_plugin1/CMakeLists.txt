cmake_minimum_required(VERSION 3.16)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/faulty_plugin1.cpp)
add_library(faulty-plugin1 MODULE ${SOURCES})
set_target_properties(
	faulty-plugin1
	PROPERTIES 
		DEFINE_SYMBOL "REX_FAULTY_PLUGIN1_EXPORTING"
)
target_link_libraries(
	faulty-plugin1
	PRIVATE
		rexlib
)
