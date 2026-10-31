# SPDX-License-Identifier: GPL-3.0-only

"""Electron microscopy operations with an optional, implicit context.

Wraps `rexlib._binding.em.functional`: same functions and parameter
order, but `context` defaults to the active one (see
`rexlib.get_active_execution_context`) instead of being required.
"""

from __future__ import annotations

from collections.abc import Sequence

from .._binding.dispatch import ExecutionContext
from .._binding.em import functional as _raw
from .._binding.ndarray import Array
from .._functional import _resolve_context

# -- preprocessing --------------------------------------------------------

def _resolve_bounds(
	bounds: float | tuple[float, float]
) -> tuple[float, float]:
	if isinstance(bounds, tuple):
		lower, upper = bounds
		return lower, upper
	return -bounds, bounds

def standardize_background(
	value: Array,
	seed: Array,
	axes: Sequence[int],
	radius: float,
	bounds: float | tuple[float, float],
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Standardize images against the statistics of their background.

	The background of an image holds its elements farther than `radius`
	from its centre, the element at index n // 2 along each of `axes`.
	Each image has the mean of its finite background subtracted and is
	divided by its standard deviation, both computed again without the
	elements whose standardized value lies outside `bounds`. Elements
	that are not finite, or that end up outside the bounds, are replaced
	by standard normal draws keyed by `seed`.

	Args:
		value: The images, of a floating point type.
		seed: The uint64 seeds of the draws. It has no image axes and
			broadcasts against the other axes of `value`: one seed for
			every image, or one per image.
		axes: The axes of an image. Negative values count from the end.
		radius: The radius of the region left out of the background, in
			elements.
		bounds: The lowest and the highest standardized value kept, on
			either side of 0, or one value `k` for the bounds `(-k, k)`.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The standardized images.
	"""
	return _raw.standardize_background(
		value,
		seed,
		list(axes),
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)

def standardize_background_2d(
	value: Array,
	seed: Array,
	radius: float,
	bounds: float | tuple[float, float],
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Standardize images held in the last two axes.

	As `standardize_background`, with the last two axes as the axes of an
	image.

	Args:
		value: The images, of a floating point type.
		seed: The uint64 seeds of the draws, one for every image or one
			per image.
		radius: The radius of the region left out of the background, in
			elements.
		bounds: The lowest and the highest standardized value kept, on
			either side of 0, or one value `k` for the bounds `(-k, k)`.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The standardized images.
	"""
	return _raw.standardize_background_2d(
		value,
		seed,
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)

def standardize_background_3d(
	value: Array,
	seed: Array,
	radius: float,
	bounds: float | tuple[float, float],
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Standardize volumes held in the last three axes.

	As `standardize_background`, with the last three axes as the axes of
	a volume.

	Args:
		value: The volumes, of a floating point type.
		seed: The uint64 seeds of the draws, one for every volume or one
			per volume.
		radius: The radius of the region left out of the background, in
			elements.
		bounds: The lowest and the highest standardized value kept, on
			either side of 0, or one value `k` for the bounds `(-k, k)`.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The standardized volumes.
	"""
	return _raw.standardize_background_3d(
		value,
		seed,
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)
