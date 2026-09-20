# SPDX-License-Identifier: GPL-3.0-only

"""Image files: where one is, and which formats are known.

`ImageLocation` addresses an element of a file, and the two format
managers hold the formats that can be read and written. Both managers are
services, reached from a `rexlib.ServiceCatalog` the way the device and
program managers are.

`read` and `write` carry the file into and out of an `Array`, taking
their format manager from the default catalog and `read` its execution
context from the active one.

A string is never parsed into an `ImageLocation` on its way anywhere: a
path handed to the constructor, or to `read`, is a path, and
`ImageLocation.from_string` is the one thing that reads `"3@stack.mrc"`
as a position in a stack. It is the inverse of `str`, and raises
`ValueError` on anything it cannot read.
"""

from __future__ import annotations

from ..._binding.em.image import (
	ImageLocation as ImageLocation,
	ImageReadFormatManager as ImageReadFormatManager,
	ImageWriteFormatManager as ImageWriteFormatManager,
	get_image_read_format_manager as get_image_read_format_manager,
	get_image_write_format_manager as get_image_write_format_manager,
)
from ._functions import (
	query_core_extents as query_core_extents,
	query_extents as query_extents,
	read as read,
	write as write,
)

__all__ = [
	"ImageLocation",
	"ImageReadFormatManager",
	"ImageWriteFormatManager",
	"get_image_read_format_manager",
	"get_image_write_format_manager",
	"query_core_extents",
	"query_extents",
	"read",
	"write",
]
