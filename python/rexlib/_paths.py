# SPDX-License-Identifier: GPL-3.0-only

"""Locate the C++ side of rexlib that this package uses."""

from __future__ import annotations

import pathlib

from ._binding import get_default_plugin_directory


def __get_library_directory() -> pathlib.Path:
	"""
	Get the directory librexlib was loaded from.

	The default plugin directory is named after it, whatever the layout.

	Returns:
		pathlib.Path: The directory.
	"""
	return pathlib.Path(get_default_plugin_directory()).resolve().parent


def get_cmake_dir() -> str:
	"""
	Get the directory holding rexlib's CMake package configuration.

	Pass it to CMake as `rexlib_DIR` to build a plugin, or anything else
	that links rexlib, against the rexlib this package uses.

	Returns:
		str: Directory containing rexlib-config.cmake.
	"""
	library = __get_library_directory()
	# On Windows the DLL is in bin, beside the prefix's library directory:
	# lib64 on 64-bit Red Hat, lib everywhere else.
	for directory in (library, library.parent / "lib64", library.parent / "lib"):
		candidate = directory / "cmake" / "rexlib"
		if candidate.is_dir():
			return str(candidate)

	raise FileNotFoundError(
		f"No rexlib CMake package configuration beside {library}"
	)


def get_include() -> str:
	"""
	Get the directory holding rexlib's C++ headers.

	Returns:
		str: Directory to add to a compiler's include path.
	"""
	library = __get_library_directory()
	for prefix in library.parents:
		candidate = prefix / "include"
		if (candidate / "rexlib").is_dir():
			return str(candidate)

	raise FileNotFoundError(f"No rexlib headers in a prefix of {library}")
