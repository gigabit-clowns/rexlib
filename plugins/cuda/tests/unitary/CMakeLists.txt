#***************************************************************************
# Authors:     Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA
#
#  All comments concerning this program package may be sent to the
#  e-mail address 'xmipp@cnb.csic.es'
# ***************************************************************************

cmake_minimum_required(VERSION 3.16)

# Test excecutable name
set(TESTS_NAME "${PROJECT_NAME}_unitary_tests")

# Register all source and header files
file(
	GLOB_RECURSE 
	TEST_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# Add the test executable
add_executable(
    ${TESTS_NAME}
    ${TEST_SOURCES}
)
set_target_properties(
	${TESTS_NAME} PROPERTIES 
	CXX_STANDARD 20
)
target_link_libraries(
	${TESTS_NAME} 
	PRIVATE 
		${PROJECT_NAME} 
		Catch2::Catch2WithMain
		trompeloeil::trompeloeil
)

# On windows targets, copy the library to the test directory
if(WIN32)
	add_custom_command(TARGET ${TESTS_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Copy DLL file to ${CMAKE_CURRENT_BINARY_DIR} directory" VERBATIM
	)
endif()

# Discover tests
catch_discover_tests(${TESTS_NAME})

# Install
install(
	TARGETS ${TESTS_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
