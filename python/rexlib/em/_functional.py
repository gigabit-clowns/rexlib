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
from .._binding.em.ops.parameters import CtfOptics, GriddingWindow
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
	key: Array,
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
	by standard normal draws keyed by `key`.

	Args:
		value: The images, of a floating point type.
		key: The uint64 keys of the draws. It has no image axes and
			broadcasts against the other axes of `value`. A key gives one
			stream of draws over every image it covers, so images sharing
			a key draw different values, and an image with a key of its
			own draws the same values however it is batched.
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
		key,
		list(axes),
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)

def standardize_background_2d(
	value: Array,
	key: Array,
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
		key: The uint64 keys of the draws, one for every image or one
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
		key,
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)

def standardize_background_3d(
	value: Array,
	key: Array,
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
		key: The uint64 keys of the draws, one for every volume or one
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
		key,
		radius,
		*_resolve_bounds(bounds),
		_resolve_context(context),
		out
	)

# -- reconstruction -------------------------------------------------------

def insert_slices(  # noqa: PLR0913, PLR0917
	numerator: Array,
	denominator: Array,
	slices: Array,
	transforms: Array,
	radial_weights: Array,
	window: GriddingWindow,
	context: ExecutionContext | None = None,
	numerator_out: Array | None = None,
	denominator_out: Array | None = None
) -> tuple[Array, Array]:
	"""Insert the spectra of projections into the spectrum of a volume.

	Each slice is the half spectrum of a projection of a volume, as `rfft2`
	gives it. It is added to a numerator, weighted by its radial weight and
	spread over the voxels by `window`, and its weight is added to a
	denominator. The numerator divided by the denominator estimates the half
	spectrum of the volume, which `irfft3` takes back to the volume. Pass
	the accumulators as `numerator_out` and `denominator_out` to update them
	in place.

	The transform `[A | t]` of a slice maps the point `p` of the volume,
	`(x, y, z)` in voxels from its centre, to the point `A p + t`, whose
	first two coordinates are the `(x, y)` of the image in pixels from its
	centre. The centre of an extent `m` is the index `m // 2`. See the
	documentation of `insert_slices_operation` for the whole contract.

	Args:
		numerator: The complex accumulator, of shape `(..., N, N, N // 2 + 1)`.
		denominator: The real accumulator, of the same shape.
		slices: The half spectra of the projections, of shape
			`(..., n, n // 2 + 1)`.
		transforms: The 3x4 transforms, of shape `(..., 3, 4)`.
		radial_weights: The weight of each frequency shell, of shape
			`(..., n // 2 + 1)`. A frequency whose shell weighs 0 is not
			inserted.
		window: The window spreading a sample over the voxels.
		context: The execution context. Defaults to the active one.
		numerator_out: An array to write the numerator into.
		denominator_out: An array to write the denominator into.

	Returns:
		The numerator and the denominator.
	"""
	return _raw.insert_slices(
		numerator,
		denominator,
		slices,
		transforms,
		radial_weights,
		window,
		_resolve_context(context),
		numerator_out,
		denominator_out
	)

def insert_weighted_slices(  # noqa: PLR0913, PLR0917
	numerator: Array,
	denominator: Array,
	slices: Array,
	weights: Array,
	transforms: Array,
	radial_weights: Array,
	window: GriddingWindow,
	context: ExecutionContext | None = None,
	numerator_out: Array | None = None,
	denominator_out: Array | None = None
) -> tuple[Array, Array]:
	"""Insert slices with a denominator weight per frequency.

	As `insert_slices`, with the denominator receiving the weight of each
	frequency of a slice rather than 1. A weight the numerator needs is
	multiplied into the slices beforehand.

	Args:
		numerator: The complex accumulator, of shape `(..., N, N, N // 2 + 1)`.
		denominator: The real accumulator, of the same shape.
		slices: The half spectra of the projections, of shape
			`(..., n, n // 2 + 1)`.
		weights: The real weight of each frequency of the slices, of shape
			`(..., n, n // 2 + 1)`.
		transforms: The 3x4 transforms, of shape `(..., 3, 4)`.
		radial_weights: The weight of each frequency shell, of shape
			`(..., n // 2 + 1)`.
		window: The window spreading a sample over the voxels.
		context: The execution context. Defaults to the active one.
		numerator_out: An array to write the numerator into.
		denominator_out: An array to write the denominator into.

	Returns:
		The numerator and the denominator.
	"""
	return _raw.insert_weighted_slices(
		numerator,
		denominator,
		slices,
		weights,
		transforms,
		radial_weights,
		window,
		_resolve_context(context),
		numerator_out,
		denominator_out
	)

def insert_ctf_slices(  # noqa: PLR0913, PLR0917
	numerator: Array,
	denominator: Array,
	slices: Array,
	transforms: Array,
	ctf: Array,
	radial_weights: Array,
	window: GriddingWindow,
	optics: CtfOptics,
	pixel_size: float,
	context: ExecutionContext | None = None,
	numerator_out: Array | None = None,
	denominator_out: Array | None = None
) -> tuple[Array, Array]:
	"""Insert slices weighted by their contrast transfer function.

	As `insert_slices`, with the numerator receiving the CTF `c(k)` times
	the slice and the denominator `c(k)^2`. The CTF is that of RELION:
	positive at low frequency, with a positive defocus for underfocus.

	Args:
		numerator: The complex accumulator, of shape `(..., N, N, N // 2 + 1)`.
		denominator: The real accumulator, of the same shape.
		slices: The half spectra of the projections, of shape
			`(..., n, n // 2 + 1)`.
		transforms: The 3x4 transforms, of shape `(..., 3, 4)`.
		ctf: The defocus along and across the astigmatism, in angstroms,
			the angle of the astigmatism from the x axis and the phase
			shift, in radians, of shape `(..., 4)`.
		radial_weights: The weight of each frequency shell, of shape
			`(..., n // 2 + 1)`.
		window: The window spreading a sample over the voxels.
		optics: The optics of the microscope.
		pixel_size: The size of a pixel of the images, in angstroms.
		context: The execution context. Defaults to the active one.
		numerator_out: An array to write the numerator into.
		denominator_out: An array to write the denominator into.

	Returns:
		The numerator and the denominator.
	"""
	return _raw.insert_ctf_slices(
		numerator,
		denominator,
		slices,
		transforms,
		ctf,
		radial_weights,
		window,
		optics,
		pixel_size,
		_resolve_context(context),
		numerator_out,
		denominator_out
	)
