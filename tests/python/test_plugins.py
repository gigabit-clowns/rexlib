# SPDX-License-Identifier: GPL-3.0-only

import os
import uuid

import pytest

import rexlib

def test_plugin_manager_constructor():
  assert rexlib.PluginManager() is not None

def test_plugin_manager_is_initialized_without_plugins():
  pm = rexlib.PluginManager()
  assert pm.plugins == []
  
def test_plugin_manager_discovers_no_plugins_in_an_empty_directory():
  pm = rexlib.PluginManager()
  pm.discover_plugins(f'/path/to/invalid/directory/{uuid.uuid4()}/')
  assert pm.plugins == []
  
def test_plugin_manager_discovers_the_whole_search_path_by_default():
  default = rexlib.PluginManager()
  default.discover_plugins()
  explicit = rexlib.PluginManager()
  for directory in rexlib.get_plugin_search_path():
    explicit.discover_plugins(directory)
  assert list(map(repr, default.plugins)) == list(map(repr, explicit.plugins))

def test_plugin_manager_discovers_plugins():
  pm = rexlib.PluginManager()
  pm.discover_plugins(__get_dummy_plugin_directory())
  plugins = set(map(repr, pm.plugins))
  assert plugins == {
    'Plugin(name="dummy-plugin1", version="1.2.3")',
    'Plugin(name="dummy-plugin2", version="4.5.6")',
  }

def test_plugin_manager_adds_a_discovered_plugin():
  source = rexlib.PluginManager()
  source.discover_plugins(__get_dummy_plugin_directory())
  target = rexlib.PluginManager()
  target.add_plugin(source.plugins[0])
  assert list(map(repr, target.plugins)) == [repr(source.plugins[0])]

def test_plugin_manager_raises_when_loading_a_missing_plugin():
  pm = rexlib.PluginManager()
  with pytest.raises(RuntimeError):
    pm.load_plugin(f'/path/to/invalid/plugin/{uuid.uuid4()}')

def test_plugin_exposes_name_and_version():
  pm = rexlib.PluginManager()
  pm.discover_plugins(__get_dummy_plugin_directory())
  plugin = next(p for p in pm.plugins if p.name == 'dummy-plugin1')
  assert plugin.version == rexlib.Version(1, 2, 3)

def test_returns_plugin_search_path():
  path = rexlib.get_plugin_search_path()
  assert isinstance(path, list)
  assert all(isinstance(entry, str) for entry in path)
  assert path[-1] == rexlib.get_default_plugin_directory()

def test_returns_default_plugin_directory():
  assert isinstance(rexlib.get_default_plugin_directory(), str)

def test_service_catalog_constructor():
  assert rexlib.ServiceCatalog() is not None

def test_service_catalog_registers_plugins_without_plugins():
  ir = rexlib.ServiceCatalog()
  pm = rexlib.PluginManager()
  n_plugins = ir.register_plugins(pm)
  assert n_plugins == 0

def __get_dummy_plugin_directory() -> str:
  # Installed beside the real plugin directory, so it is found the same
  # way whatever prefix layout the package was built with.
  parent = os.path.dirname(rexlib.get_default_plugin_directory())
  return os.path.join(parent, 'rexlib-dummy-plugins')
