# SPDX-License-Identifier: GPL-3.0-only

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/faulty_plugin2.cpp)
add_library(faulty-plugin2 MODULE ${SOURCES})
target_link_libraries(
	faulty-plugin2
	PRIVATE
		rexlib
)
