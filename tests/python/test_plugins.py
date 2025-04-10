#***************************************************************************
# Authors:     Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
#              Martín Salinas Antón (ssalinasmartin@gmail.com)
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

import os
import platform
import sys
import uuid

import xmipp4

def test_plugin_manager_constructor():
  assert xmipp4.PluginManager() is not None

def test_plugin_manager_is_initialized_without_plugins():
  pm = xmipp4.PluginManager()
  assert pm.plugins == []
  
def test_plugin_manager_discovers_no_plugins_in_an_empty_directory():
  pm = xmipp4.PluginManager()
  pm.discover_plugins(f'/path/to/invalid/directory/{uuid.uuid4()}/')
  assert pm.plugins == []
  
def test_plugin_manager_discovers_plugins():
  pm = xmipp4.PluginManager()
  pm.discover_plugins(__get_dummy_plugin_directory())
  plugins = set(map(repr, pm.plugins))
  plugins == {
    'Plugin(name="dummy-plugin1", version="1.2.3")',
    'Plugin(name="dummy-plugin2", version="4.5.6")',
  }

def test_interface_catalog_constructor():
  assert xmipp4.InterfaceCatalog() is not None

def test_interface_catalog_registers_plugins_without_plugins():
  ir = xmipp4.InterfaceCatalog()
  pm = xmipp4.PluginManager()
  n_plugins = ir.register_plugins(pm)
  assert n_plugins == 0

def __get_dummy_plugin_directory() -> str:
  prefix = sys.prefix
  DUMMY_PLUGIN_DIRECTORY_NAME = 'xmipp4-dummy-plugins'
  
  if platform.system() == 'Windows':
    return os.path.join(prefix, 'bin', DUMMY_PLUGIN_DIRECTORY_NAME)
  elif os.path.exists(os.path.join(prefix, 'lib')):
    return os.path.join(prefix, 'lib', DUMMY_PLUGIN_DIRECTORY_NAME)
  elif os.path.exists(os.path.join(prefix, 'lib64')):
    return os.path.join(prefix, 'lib64', DUMMY_PLUGIN_DIRECTORY_NAME)
  else:
    raise OSError("Could not find the dummy plugin directory.")
