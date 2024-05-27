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

include(FetchContent)
include(GNUInstallDirs)

# Find requirements
find_package(Python COMPONENTS Interpreter Development REQUIRED)
FetchContent_Declare(
	pybind11
	GIT_REPOSITORY https://github.com/pybind/pybind11
	GIT_TAG        v2.12.0
)
FetchContent_MakeAvailable(pybind11)

# Find xmipp4-core
if (NOT TARGET xmipp4-core)
	find_package(xmipp4-core REQUIRED)
endif()

# Register all source and header files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE INLINE_SOURCES ${PROJECT_SOURCE_DIR}/include/*.inl)

# Create the shared library
pybind11_add_module(_core SHARED ${SOURCES})
set_target_properties(
	_core PROPERTIES 
	CXX_STANDARD 20
)
target_compile_definitions(
	_core
	PRIVATE
		VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
		VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
		VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
)
target_precompile_headers(
	_core
	PRIVATE ${HEADERS}
)
target_link_libraries(
	_core
	PRIVATE xmipp4-core
)
if(MSVC)
	target_compile_options(_core PRIVATE /W4 /WX)
else()
	target_compile_options(_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Combine with Python source
set(PYTHON_SOURCE_DIR ${PROJECT_SOURCE_DIR}/python)
add_custom_command(
	TARGET _core POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYTHON_SOURCE_DIR} ${PROJECT_BINARY_DIR}/python
)
add_custom_command(
	TARGET _core POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_core> ${PROJECT_BINARY_DIR}/python/xmipp4
)

# Install library's binary files and headers
install(
	TARGETS _core
	LIBRARY 
	DESTINATION xmipp4
)
