"""Locate the C++ side of rexlib that ships inside this package."""

from __future__ import annotations

import os
import pathlib
import sys

# The wheel nests a rexlib install prefix in the package directory, so
# these are the paths a standalone install exposes, rooted here.
__PREFIX = pathlib.Path(__file__).parent

# Which of these the prefix uses is the platform's choice, made when the
# library was built: 64-bit Red Hat installs into lib64, everything else
# into lib. Both are looked for rather than one being imposed.
__LIBRARY_DIRECTORIES = (__PREFIX / "lib64", __PREFIX / "lib")

# Windows has no RPATH, and the shared library does not sit beside the
# extension that needs it, so the loader has to be told where to look.
# Kept alive for the life of the process: dropping the handle takes the
# directory back off the search path.
__dll_directories = [
	os.add_dll_directory(str(directory))
	for directory in (__PREFIX / "bin", *__LIBRARY_DIRECTORIES)
	if sys.platform == "win32" and directory.is_dir()
]


def get_cmake_dir() -> str:
	"""
	Get the directory holding rexlib's CMake package configuration.

	Pass it to CMake as `rexlib_DIR` to build a plugin, or anything else
	that links rexlib, against the copy shipped in this package.

	Returns:
		str: Directory containing rexlib-config.cmake.
	"""
	for directory in __LIBRARY_DIRECTORIES:
		candidate = directory / "cmake" / "rexlib"
		if candidate.is_dir():
			return str(candidate)

	raise FileNotFoundError(
		f"No rexlib CMake package configuration under {__PREFIX}"
	)


def get_include() -> str:
	"""
	Get the directory holding rexlib's C++ headers.

	Returns:
		str: Directory to add to a compiler's include path.
	"""
	return str(__PREFIX / "include")
