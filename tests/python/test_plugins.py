# SPDX-License-Identifier: GPL-3.0-only

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
  assert plugins == {
    'Plugin(name="dummy-plugin1", version="1.2.3")',
    'Plugin(name="dummy-plugin2", version="4.5.6")',
  }

def test_service_catalog_constructor():
  assert xmipp4.ServiceCatalog() is not None

def test_service_catalog_registers_plugins_without_plugins():
  ir = xmipp4.ServiceCatalog()
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
