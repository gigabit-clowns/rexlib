# SPDX-License-Identifier: GPL-3.0-only

"""Reading and writing image files, with the managers defaulted.

Wraps `rexlib._binding.em.image`: same functions and parameter order,
but `manager` defaults to the one the default catalog holds (see
`rexlib.get_default_catalog`) and `read`'s `context` to the active
execution context, the way `rexlib.add` and its siblings take theirs.
"""

from __future__ import annotations

from ..._binding.dispatch import ExecutionContext
from ..._binding.em import image as _raw
from ..._binding.em.image import (
	ImageLocation,
	ImageReadFormatManager,
	ImageWriteFormatManager,
	get_image_read_format_manager,
	get_image_write_format_manager,
)
from ..._binding.ndarray import Array
from ..._binding.numerical import NumericalType
from ..._catalog import get_default_catalog
from ..._functional import _resolve_context

def _resolve_read_manager(
	manager: ImageReadFormatManager | None
) -> ImageReadFormatManager:
	if manager is None:
		manager = get_image_read_format_manager(get_default_catalog())
	return manager

def _resolve_write_manager(
	manager: ImageWriteFormatManager | None
) -> ImageWriteFormatManager:
	if manager is None:
		manager = get_image_write_format_manager(get_default_catalog())
	return manager

def read(
	path: str | ImageLocation,
	manager: ImageReadFormatManager | None = None,
	context: ExecutionContext | None = None
) -> Array:
	"""
	Read an image file into an array.

	The array is allocated on the host whatever device is active, so
	reaching a device with it is an explicit `rexlib.to_device`. An
	execution context is still required, the same one array creation
	takes, so a bare read outside `with rexlib.device(...):` raises.

	Args:
		path: The file to read, or an `ImageLocation` addressing one
			element of it. A string is a path and only a path; read
			`"3@stack.mrc"` by handing it to `ImageLocation.from_string`
			first.
		manager: The formats to recognize the file with. Defaults to the
			ones the default catalog holds.
		context: The execution context to allocate under. Defaults to the
			active one.

	Returns:
		Array: The contents of the file, on the host.
	"""
	return _raw.read(path, _resolve_read_manager(manager), _resolve_context(context))

def write(
	array: Array,
	path: str,
	manager: ImageWriteFormatManager | None = None,
	data_type: NumericalType | None = None
) -> None:
	"""
	Write an array to an image file.

	The array has to be reachable from the host; writing one that lives
	on a device raises rather than transferring it, so the transfer is
	an explicit `rexlib.to_host`.

	Args:
		array: The values to write.
		path: The file to write them to.
		manager: The formats to choose from. Defaults to the ones the
			default catalog holds.
		data_type: The type to store the values as. Defaults to the one
			the array already carries.
	"""
	_raw.write(array, path, _resolve_write_manager(manager), data_type)
