# SPDX-License-Identifier: GPL-3.0-only

"""Load the plugins that installed distributions declare."""

from __future__ import annotations

import importlib.metadata
import importlib.util
import pathlib
import sys
import warnings

from ._binding import PluginManager

__ENTRY_POINT_GROUP = "rexlib.plugins"
__PLUGIN_DIRECTORY_NAME = "rexlib-plugins"
__SHARED_OBJECT_SUFFIXES = {
	"win32": (".dll",),
	"darwin": (".so", ".dylib"),
}


def __get_shared_object_suffixes() -> tuple[str, ...]:
	"""
	Get the suffixes a plugin's shared object has on this platform.

	Returns:
		tuple[str, ...]: The suffixes, with their leading dot.
	"""
	return __SHARED_OBJECT_SUFFIXES.get(sys.platform, (".so",))


def __find_package_directories(package: str) -> list[pathlib.Path]:
	"""
	Find the directories of a package without importing it.

	Args:
		package: The name of the package.

	Returns:
		list[pathlib.Path]: The directories the package spans.

	Raises:
		ModuleNotFoundError: If no package of that name is installed.
	"""
	spec = importlib.util.find_spec(package)
	if spec is None or spec.submodule_search_locations is None:
		message = f"No package named {package!r} is installed"
		raise ModuleNotFoundError(message, name=package)
	return [
		pathlib.Path(location)
		for location in spec.submodule_search_locations
	]


def __find_plugin_files(
	directories: list[pathlib.Path], suffixes: tuple[str, ...]
) -> list[pathlib.Path]:
	"""
	Find the shared objects in the plugin directories of a package.

	Args:
		directories: The directories the package spans.
		suffixes: The suffixes a shared object has on this platform.

	Returns:
		list[pathlib.Path]: The shared objects directly in a directory named
		`rexlib-plugins`, at any depth, sorted.
	"""
	return sorted(
		path
		for directory in directories
		for plugins in directory.rglob(__PLUGIN_DIRECTORY_NAME)
		if plugins.is_dir()
		for path in plugins.iterdir()
		if path.suffix in suffixes and path.is_file()
	)


def discover_entry_point_plugins(manager: PluginManager) -> None:
	"""
	Load the plugins that installed distributions declare.

	A distribution declares a plugin with an entry point in the group
	`rexlib.plugins` whose value names a package, which holds the plugin as
	an install prefix does: in a directory named `rexlib-plugins`. Every
	shared object directly in such a directory, at any depth of the package,
	is loaded into the manager as a plugin. A package that is not installed,
	or a shared object the manager refuses, is skipped with a
	`RuntimeWarning`.

	Args:
		manager: The plugin manager the plugins are loaded into.
	"""
	suffixes = __get_shared_object_suffixes()
	entry_points = importlib.metadata.entry_points(group=__ENTRY_POINT_GROUP)
	for entry_point in entry_points:
		try:
			directories = __find_package_directories(entry_point.module)
		except ImportError as error:
			warnings.warn(
				f"rexlib skipped the plugins of {entry_point.name!r}: {error}",
				RuntimeWarning,
				stacklevel=2,
			)
			continue

		for path in __find_plugin_files(directories, suffixes):
			try:
				manager.load_plugin(str(path))
			except RuntimeError as error:
				warnings.warn(
					f"rexlib skipped the plugin {path}, which "
					f"{entry_point.name!r} declares: {error}",
					RuntimeWarning,
					stacklevel=2,
				)
