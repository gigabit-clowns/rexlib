# SPDX-License-Identifier: GPL-3.0-only

import os
import platform
import sys
import uuid

import pytest

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
  
def test_plugin_manager_discovers_in_the_plugin_directory_by_default():
  default = xmipp4.PluginManager()
  default.discover_plugins()
  explicit = xmipp4.PluginManager()
  explicit.discover_plugins(xmipp4.get_plugin_directory())
  assert list(map(repr, default.plugins)) == list(map(repr, explicit.plugins))

def test_plugin_manager_discovers_plugins():
  pm = xmipp4.PluginManager()
  pm.discover_plugins(__get_dummy_plugin_directory())
  plugins = set(map(repr, pm.plugins))
  assert plugins == {
    'Plugin(name="dummy-plugin1", version="1.2.3")',
    'Plugin(name="dummy-plugin2", version="4.5.6")',
  }

def test_plugin_manager_adds_a_discovered_plugin():
  source = xmipp4.PluginManager()
  source.discover_plugins(__get_dummy_plugin_directory())
  target = xmipp4.PluginManager()
  target.add_plugin(source.plugins[0])
  assert list(map(repr, target.plugins)) == [repr(source.plugins[0])]

def test_plugin_manager_raises_when_loading_a_missing_plugin():
  pm = xmipp4.PluginManager()
  with pytest.raises(RuntimeError):
    pm.load_plugin(f'/path/to/invalid/plugin/{uuid.uuid4()}')

def test_plugin_exposes_name_and_version():
  pm = xmipp4.PluginManager()
  pm.discover_plugins(__get_dummy_plugin_directory())
  plugin = next(p for p in pm.plugins if p.name == 'dummy-plugin1')
  assert plugin.version == xmipp4.Version(1, 2, 3)

def test_returns_plugin_directory():
  assert isinstance(xmipp4.get_plugin_directory(), str)

def test_returns_default_plugin_directory():
  assert isinstance(xmipp4.get_default_plugin_directory(), str)

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
  if os.path.exists(os.path.join(prefix, 'lib')):
    return os.path.join(prefix, 'lib', DUMMY_PLUGIN_DIRECTORY_NAME)
  if os.path.exists(os.path.join(prefix, 'lib64')):
    return os.path.join(prefix, 'lib64', DUMMY_PLUGIN_DIRECTORY_NAME)
  raise OSError("Could not find the dummy plugin directory.")
