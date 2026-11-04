# SPDX-License-Identifier: GPL-3.0-only

import os
import pathlib
import shutil
import subprocess
import sys
import uuid
import warnings

import pytest

import rexlib

__SHARED_OBJECT_SUFFIXES = {".so", ".dylib", ".dll"}

def test_discover_entry_point_plugins_loads_the_plugins_a_package_holds(
	monkeypatch, tmp_path
):
	package = __make_package_name()
	__copy_dummy_plugins(tmp_path / package / "lib" / "rexlib-plugins")
	__install_distribution(monkeypatch, tmp_path, {"dummy": package})
	manager = rexlib.PluginManager()

	rexlib.discover_entry_point_plugins(manager)

	assert {"dummy-plugin1", "dummy-plugin2"} <= __get_names(manager)

def test_discover_entry_point_plugins_ignores_plugins_outside_their_directory(
	monkeypatch, tmp_path
):
	package = __make_package_name()
	__copy_dummy_plugins(tmp_path / package)
	__install_distribution(monkeypatch, tmp_path, {"dummy": package})
	manager = rexlib.PluginManager()

	with warnings.catch_warnings():
		warnings.simplefilter("error")
		rexlib.discover_entry_point_plugins(manager)

	assert manager.plugins == []

def test_discover_entry_point_plugins_ignores_files_that_are_no_shared_object(
	monkeypatch, tmp_path
):
	package = __make_package_name()
	plugins = tmp_path / package / "lib" / "rexlib-plugins"
	__copy_dummy_plugins(plugins)
	(tmp_path / package / "__init__.py").write_text("")
	(plugins / "README.txt").write_text("Not a plugin")
	__install_distribution(monkeypatch, tmp_path, {"dummy": package})
	manager = rexlib.PluginManager()

	with warnings.catch_warnings():
		warnings.simplefilter("error")
		rexlib.discover_entry_point_plugins(manager)

	assert {"dummy-plugin1", "dummy-plugin2"} <= __get_names(manager)

def test_discover_entry_point_plugins_warns_of_a_shared_object_it_cannot_load(
	monkeypatch, tmp_path
):
	package = __make_package_name()
	plugins = tmp_path / package / "lib" / "rexlib-plugins"
	suffix = __copy_dummy_plugins(plugins)
	broken = plugins / f"broken{suffix}"
	broken.write_bytes(b"Not a shared object")
	__install_distribution(monkeypatch, tmp_path, {"dummy": package})
	manager = rexlib.PluginManager()

	with pytest.warns(RuntimeWarning, match="broken"):
		rexlib.discover_entry_point_plugins(manager)

	assert {"dummy-plugin1", "dummy-plugin2"} <= __get_names(manager)

def test_discover_entry_point_plugins_warns_of_a_package_not_installed(
	monkeypatch, tmp_path
):
	package = __make_package_name()
	__install_distribution(monkeypatch, tmp_path, {"missing": package})
	manager = rexlib.PluginManager()

	with pytest.warns(RuntimeWarning, match=package):
		rexlib.discover_entry_point_plugins(manager)

def test_discover_entry_point_plugins_warns_of_a_plugin_already_loaded(
	monkeypatch, tmp_path
):
	first = __make_package_name()
	second = __make_package_name()
	__copy_dummy_plugins(tmp_path / first / "lib" / "rexlib-plugins")
	__copy_dummy_plugins(tmp_path / second / "lib" / "rexlib-plugins")
	__install_distribution(
		monkeypatch, tmp_path, {"first": first, "second": second}
	)
	manager = rexlib.PluginManager()

	with pytest.warns(RuntimeWarning, match="already"):
		rexlib.discover_entry_point_plugins(manager)

	names = [p.name for p in manager.plugins]
	assert names.count("dummy-plugin1") == 1

def test_default_catalog_discovers_entry_point_plugins(tmp_path):
	package = __make_package_name()
	__write_distribution(tmp_path, {"missing": package})
	path = os.pathsep.join(
		[str(tmp_path), *filter(None, [os.environ.get("PYTHONPATH")])]
	)
	code = (
		"import warnings, rexlib\n"
		"with warnings.catch_warnings(record=True) as caught:\n"
		"    warnings.simplefilter('always')\n"
		"    rexlib.get_default_catalog()\n"
		"print('\\n'.join(str(w.message) for w in caught))\n"
	)

	result = subprocess.run(
		[sys.executable, "-c", code],
		env={**os.environ, "PYTHONPATH": path},
		capture_output=True,
		text=True,
		check=True,
	)

	assert package in result.stdout

def __make_package_name() -> str:
	return f"rexlib_test_plugins_{uuid.uuid4().hex}"

def __copy_dummy_plugins(directory: pathlib.Path) -> str:
	directory.mkdir(parents=True)
	source = pathlib.Path(os.environ["REXLIB_PYTHON_TEST_PLUGINS_DIR"])
	suffix = ""
	for path in source.iterdir():
		if path.suffix in __SHARED_OBJECT_SUFFIXES:
			shutil.copy(path, directory / path.name)
			suffix = path.suffix
	return suffix

def __write_distribution(
	directory: pathlib.Path, entry_points: dict[str, str]
) -> None:
	name = f"rexlib_test_distribution_{uuid.uuid4().hex}"
	dist_info = directory / f"{name}-0.0.0.dist-info"
	dist_info.mkdir()
	(dist_info / "METADATA").write_text(
		f"Metadata-Version: 2.1\nName: {name}\nVersion: 0.0.0\n"
	)
	lines = [f"{key} = {value}" for key, value in entry_points.items()]
	(dist_info / "entry_points.txt").write_text(
		"[rexlib.plugins]\n" + "\n".join(lines) + "\n"
	)

def __install_distribution(
	monkeypatch, directory: pathlib.Path, entry_points: dict[str, str]
) -> None:
	__write_distribution(directory, entry_points)
	monkeypatch.syspath_prepend(str(directory))

def __get_names(manager: rexlib.PluginManager) -> set[str]:
	return {plugin.name for plugin in manager.plugins}
