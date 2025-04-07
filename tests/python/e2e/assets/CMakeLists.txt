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

add_subdirectory(dummy_plugin1)
add_subdirectory(dummy_plugin2)

set(TARGET_NAME "${PROJECT_NAME}_e2e_test_assets")
set(PLUGINS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins/)
add_custom_target(
    ${TARGET_NAME}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGINS_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_plugin1> ${PLUGINS_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dummy_plugin2> ${PLUGINS_DIRECTORY}
)
