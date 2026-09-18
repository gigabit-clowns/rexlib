# SPDX-License-Identifier: GPL-3.0-only

"""Image files: where one is, and which formats are known.

`ImageLocation` addresses an element of a file, and the two format
managers hold the formats that can be read and written. Both managers are
services, reached from a `rexlib.ServiceCatalog` the way the device and
program managers are.

A string is never parsed into an `ImageLocation` on its way anywhere: a
path is a path, and `parse_image_location` is asked for by name.
"""

from __future__ import annotations

from ..._binding.em.image import (
	ImageLocation as ImageLocation,
	ImageReadFormatManager as ImageReadFormatManager,
	ImageWriteFormatManager as ImageWriteFormatManager,
	get_image_read_format_manager as get_image_read_format_manager,
	get_image_write_format_manager as get_image_write_format_manager,
	parse_image_location as parse_image_location,
)

__all__ = [
	"ImageLocation",
	"ImageReadFormatManager",
	"ImageWriteFormatManager",
	"get_image_read_format_manager",
	"get_image_write_format_manager",
	"parse_image_location",
]
