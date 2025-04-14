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
install(
	TARGETS ${TARGET_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/xmipp4-dummy-plugins/
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/xmipp4-dummy-plugins/
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/xmipp4-dummy-plugins/
)
