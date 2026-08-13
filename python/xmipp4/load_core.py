"""Locate and load xmipp4-core's shared library at import time."""

import ctypes
import os
import platform
import sys
from collections.abc import Generator
from importlib import metadata

def __get_library_filename(name: str, system: str) -> str:
	"""Get the library filename based on the operating system."""
	if system == 'Darwin':
		return f"lib{name}.dylib"
	if system == 'Windows':
		return f"{name}.dll"
	# Linux or other Unix-like systems
	return f"lib{name}.so"

def __get_library_directory_names(system) -> list[str]:
	"""Get the library directory names based on the operating system."""
	if system == 'Windows':
		return ["bin"]
	# Linux, MacOS, or other Unix-like systems
	return ["lib", "lib64"]

def _iter_distribution_library_paths(
		name: str,
		filename: str
	) -> Generator[str, None, None]:
	"""
	Iterate over the paths the distribution shipping the library recorded.

	The library is installed through the wheel's data scheme, so its
	location relative to the distribution is known and does not depend on
	the layout of the environment. This is what finds it under a build
	isolation directory, a `pip install --target` tree, or any other
	prefix that is not the running interpreter's.
	"""
	try:
		distribution = metadata.distribution(name)
	except metadata.PackageNotFoundError:
		return

	# None when the installation records no file list, as is the case for
	# some conda packages.
	for file in distribution.files or ():
		if file.name == filename:
			path = os.path.normpath(str(distribution.locate_file(file)))
			if os.path.exists(path):
				yield path

def __iter_possible_library_paths(
		name: str,
		prefix: str,
		filename: str,
		system: str
	) -> Generator[str, None, None]:
	"""Iterate over possible paths for the library."""
	yield filename
	yield from _iter_distribution_library_paths(name, filename)
	for libdir in __get_library_directory_names(system):
		path = os.path.join(prefix, libdir, filename)
		if os.path.exists(path):
			yield path

def __load_library(name: str) -> ctypes.CDLL:
	"""Heuristically find and load a library with the specified name."""
	system = platform.system()
	filename = __get_library_filename(name, system)
	paths = __iter_possible_library_paths(name, sys.prefix, filename, system)
	for dynamic_lib in paths:
		try:
			return ctypes.CDLL(dynamic_lib)
		except OSError:  # noqa: PERF203 -- a handful of candidates, not a hot path
			continue
	
	raise OSError(f"Could not find {name}.")

def load_core() -> ctypes.CDLL:
	"""
	Load the core library for xmipp4.
	
	This function attempts to load the xmipp4 shared object from the system's
	library directories. It raises an exception if the library cannot be found.
	
	Returns:
		ctypes.CDLL: The loaded core library.
	"""
	return __load_library("xmipp4-core")
