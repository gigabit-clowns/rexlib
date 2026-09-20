# SPDX-License-Identifier: GPL-3.0-only

"""Reading and writing image files, with the managers defaulted.

Wraps `rexlib._binding.em.image`: same functions and parameter order,
but `manager` defaults to the one the default catalog holds (see
`rexlib.get_default_catalog`) and `read`'s `context` to the active
execution context, the way `rexlib.add` and its siblings take theirs.
"""

from __future__ import annotations

import os

from ..._binding.concurrency import Executor, ThreadPoolExecutor
from ..._binding.dispatch import ExecutionContext
from ..._binding.em import image as _raw
from ..._binding.em.image import (
	CachingImageReaderProvider,
	DirectImageReaderProvider,
	ImageBatchSource,
	ImageLocation,
	ImageSource,
	ImageReadFormatManager,
	ImageWriteFormatManager,
	get_image_read_format_manager,
	get_image_write_format_manager,
)
from ..._binding.ndarray import Array
from ..._binding.numerical import NumericalType
from ..._catalog import get_default_catalog
from ..._functional import _resolve_context

def _default_worker_count() -> int:
	return os.cpu_count() or 1

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

def batch_source(
	workers: int | None = None,
	cache: int | None = None,
	manager: ImageReadFormatManager | None = None,
	executor: Executor | None = None
) -> ImageBatchSource:
	"""
	Assemble a batch source over a thread pool.

	Wires the four objects a batch source stands on — the formats, a
	reader provider, an executor and the source itself — so that a
	caller reaches the pipeline without naming them.

	Each source built this way owns its executor. Two of them means two
	thread pools, each sized to the machine, which is worth avoiding by
	passing one executor to both.

	Args:
		workers: How many threads to run reads on. Defaults to what the
			machine reports. Ignored when `executor` is given.
		cache: How many open readers to keep between reads. Defaults to
			opening a file every time it is asked for, which is what
			suits reads that do not revisit a file.
		manager: The formats to recognize files with. Defaults to the
			ones the default catalog holds.
		executor: Where reads run. Defaults to a thread pool of its own.

	Returns:
		ImageBatchSource: The assembled source.
	"""
	readers = DirectImageReaderProvider(_resolve_read_manager(manager))
	if cache is not None:
		readers = CachingImageReaderProvider(readers, cache)
	if executor is None:
		executor = ThreadPoolExecutor(
			workers if workers is not None else _default_worker_count()
		)
	return ImageBatchSource(ImageSource(readers, executor))

def query_extents(
	path: str,
	manager: ImageReadFormatManager | None = None
) -> tuple[int, ...]:
	"""
	Get the extents of a whole image file.

	Slowest axis first, so a stack reports the axis it stacks along
	before the shape of one of its images.

	Args:
		path: The file to ask about.
		manager: The formats to recognize the file with. Defaults to the
			ones the default catalog holds.

	Returns:
		tuple: The extents of the file.
	"""
	return _raw.query_extents(_resolve_read_manager(manager), path)

def query_core_extents(
	path: str,
	manager: ImageReadFormatManager | None = None
) -> tuple[int, ...]:
	"""
	Get the extents of one image or volume of a file.

	The axes a file stacks along are left out, so this is the shape a
	batch destination carries beside its leading extent:

		descriptor = rexlib.make_contiguous_array_descriptor(
			(len(locations), *query_core_extents(path)), data_type
		)

	Args:
		path: The file to ask about.
		manager: The formats to recognize the file with. Defaults to the
			ones the default catalog holds.

	Returns:
		tuple: The extents of one image or volume.
	"""
	return _raw.query_core_extents(_resolve_read_manager(manager), path)

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
