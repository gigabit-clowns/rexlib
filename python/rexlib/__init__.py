# SPDX-License-Identifier: GPL-3.0-only

"""Python bindings for rexlib.

Everything below is private. `_binding` mirrors the C++ API one module
per namespace, with the same names and the same required arguments; the
modules beside it add what Python expects on top. This file is the whole
public surface.

Operations and the types they work on are here rather than in
sub-namespaces, matching both the C++ side, where they are in `rexlib`
itself, and the shape of the array libraries they are used alongside.
Device management and dispatch stay in their own modules: they are
reached for when setting up rather than when computing.
"""

from __future__ import annotations

# Imported first: on Windows it puts the bundled library on the DLL
# search path, which importing the extension depends on.
from . import _paths as _paths

from ._binding import (
	dispatch as dispatch,
	hardware as hardware,
	Plugin as Plugin,
	PluginManager as PluginManager,
	ServiceCatalog as ServiceCatalog,
	Version as Version,
	get_default_plugin_directory as get_default_plugin_directory,
	get_plugin_search_path as get_plugin_search_path,
	rexlib_binding_version as rexlib_binding_version,
	rexlib_version as rexlib_version,
)
from ._binding.numerical import NumericalType as NumericalType

from ._catalog import get_default_catalog as get_default_catalog
from ._context import get_active_execution_context as get_active_execution_context
from ._device import device as device
from ._paths import get_cmake_dir as get_cmake_dir, get_include as get_include

# Imported for its effect: it installs the Python operators onto Array,
# which has to happen before anything hands one out.
from . import _ndarray as _ndarray

from ._binding.ndarray import (
	Array as Array,
	ArrayDescriptor as ArrayDescriptor,
	is_initialized as is_initialized,
	make_contiguous_array_descriptor as make_contiguous_array_descriptor,
)
from ._functional import (
	abs as abs,
	add as add,
	cast as cast,
	cast_copy as cast_copy,
	copy as copy,
	divide as divide,
	empty as empty,
	fill as fill,
	full as full,
	modulo as modulo,
	multiply as multiply,
	negate as negate,
	ones as ones,
	subtract as subtract,
	to_device as to_device,
	to_device_copy as to_device_copy,
	to_host as to_host,
	to_host_copy as to_host_copy,
	transfer as transfer,
	transfer_copy as transfer_copy,
	zeros as zeros,
)

__all__ = [
	"Array",
	"ArrayDescriptor",
	"Plugin",
	"PluginManager",
	"ServiceCatalog",
	"Version",
	"__version__",
	"abs",
	"add",
	"cast",
	"cast_copy",
	"copy",
	"device",
	"dispatch",
	"divide",
	"empty",
	"fill",
	"full",
	"get_active_execution_context",
	"get_cmake_dir",
	"get_default_catalog",
	"get_default_plugin_directory",
	"get_include",
	"get_plugin_search_path",
	"hardware",
	"is_initialized",
	"make_contiguous_array_descriptor",
	"modulo",
	"multiply",
	"negate",
	"ones",
	"rexlib_binding_version",
	"rexlib_version",
	"subtract",
	"to_device",
	"to_device_copy",
	"to_host",
	"to_host_copy",
	"transfer",
	"transfer_copy",
	"zeros",
]

__version__ = str(rexlib_binding_version)
