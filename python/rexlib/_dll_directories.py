# SPDX-License-Identifier: GPL-3.0-only

"""Put the rexlib DLL on the search path, on Windows."""

from __future__ import annotations

import os
import pathlib
import sys
import sysconfig

__PACKAGE = pathlib.Path(__file__).parent


def __get_default_prefix() -> pathlib.Path | None:
	"""
	Get the install prefix the package sits under in the default layout.

	That is the platform library directory the interpreter's install scheme
	names under the prefix, where the build installs the package unless told
	otherwise.

	Returns:
		pathlib.Path | None: The prefix, or None for any other layout.
	"""
	platlib = pathlib.PurePath(
		sysconfig.get_path("platlib", vars={"base": "", "platbase": ""})
	)
	parts = platlib.relative_to(platlib.anchor).parts
	site = __PACKAGE.parent
	if not parts or site.parts[-len(parts):] != parts:
		return None
	return site.parents[len(parts) - 1]


def __get_dll_directories() -> list[pathlib.Path]:
	"""
	Get the directories the rexlib DLL is looked for in.

	The package directory is itself the prefix in a wheel, and sits under it
	in the default layout. Elsewhere the DLL has to be on the search path
	already.

	Returns:
		list[pathlib.Path]: The bin directories of those prefixes that exist.
	"""
	prefixes = [__PACKAGE, __get_default_prefix()]
	return [
		prefix / "bin"
		for prefix in prefixes
		if prefix is not None and (prefix / "bin").is_dir()
	]


# Windows has no RPATH, and the shared library does not sit beside the
# extension that needs it, so the loader is told where to look before the
# extension is imported. Kept alive for the life of the process: dropping
# the handle takes the directory back off the search path.
__dll_directories = (
	[os.add_dll_directory(str(d)) for d in __get_dll_directories()]
	if sys.platform == "win32"
	else []
)
