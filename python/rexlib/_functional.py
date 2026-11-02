# SPDX-License-Identifier: GPL-3.0-only

"""Array operations with an optional, implicit execution context.

Wraps `rexlib._binding.functional`: same functions and parameter
order, but `context` defaults to the active one (see
`rexlib.get_active_execution_context`) instead of being required.
"""

from __future__ import annotations
from collections.abc import Sequence
from typing import Any

from ._binding import functional as _raw
from ._binding.ndarray import Array, ArrayDescriptor
from ._binding.numerical import NumericalType
from ._binding.hardware import MemoryResourceAffinity
from ._binding.dispatch import ExecutionContext
from ._binding.ops.fourier import FourierNormalization
from ._context import get_active_execution_context

def _resolve_context(context: ExecutionContext | None) -> ExecutionContext:
	if context is None:
		context = get_active_execution_context()
		if context.device_context.device_session is None:
			raise RuntimeError(
				"No execution context was provided and there is no active "
				"device context. Pass context= explicitly or use "
				"'with rexlib.device(...):'."
			)
	return context

# -- arithmetic -----------------------------------------------------------

def add(
	x: Array, y: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise sum of two arrays."""
	return _raw.add(x, y, _resolve_context(context), out)

def subtract(
	x: Array, y: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise difference of two arrays."""
	return _raw.subtract(x, y, _resolve_context(context), out)

def negate(
	x: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise negation of an array."""
	return _raw.negate(x, _resolve_context(context), out)

def abs(
	x: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise absolute value of an array."""
	return _raw.abs(x, _resolve_context(context), out)

def multiply(
	x: Array, y: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise product of two arrays."""
	return _raw.multiply(x, y, _resolve_context(context), out)

def divide(
	x: Array, y: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise quotient of two arrays."""
	return _raw.divide(x, y, _resolve_context(context), out)

def modulo(
	x: Array, y: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Compute the element-wise remainder of dividing two arrays."""
	return _raw.modulo(x, y, _resolve_context(context), out)

# -- cast -------------------------------------------------------------------

def cast(
	input: Array, target_type: NumericalType,
	context: ExecutionContext | None = None
) -> Array:
	"""Cast an array to a different numerical type in place."""
	return _raw.cast(input, target_type, _resolve_context(context))

def cast_copy(
	input: Array, target_type: NumericalType,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Cast an array to a different numerical type, returning a copy."""
	return _raw.cast_copy(input, target_type, _resolve_context(context), out)

# -- transfer -----------------------------------------------------------

def transfer(
	input: Array, affinity: MemoryResourceAffinity,
	context: ExecutionContext | None = None
) -> Array:
	"""Move an array to a different memory resource affinity in place."""
	return _raw.transfer(input, affinity, _resolve_context(context))

def transfer_copy(
	input: Array, affinity: MemoryResourceAffinity,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Move an array to a different memory resource affinity, returning a copy."""
	return _raw.transfer_copy(input, affinity, _resolve_context(context), out)

def to_device(
	input: Array,
	context: ExecutionContext | None = None
) -> Array:
	"""Move an array to the device's memory resource affinity in place."""
	return _raw.to_device(input, _resolve_context(context))

def to_device_copy(
	input: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Move an array to the device's memory resource affinity, returning a copy."""
	return _raw.to_device_copy(input, _resolve_context(context), out)

def to_host(
	input: Array,
	context: ExecutionContext | None = None
) -> Array:
	"""Move an array to the host's memory resource affinity in place."""
	return _raw.to_host(input, _resolve_context(context))

def to_host_copy(
	input: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Move an array to the host's memory resource affinity, returning a copy."""
	return _raw.to_host_copy(input, _resolve_context(context), out)

# -- creation -----------------------------------------------------------

def empty(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Create an array with uninitialized contents."""
	return _raw.empty(descriptor, affinity, _resolve_context(context), out)

def zeros(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Create an array filled with zeros."""
	return _raw.zeros(descriptor, affinity, _resolve_context(context), out)

def ones(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Create an array filled with ones."""
	return _raw.ones(descriptor, affinity, _resolve_context(context), out)

def full(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity, fill_value: Any,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Create an array filled with the given value."""
	return _raw.full(descriptor, affinity, fill_value, _resolve_context(context), out)

def copy(
	source: Array,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Create a copy of an array."""
	return _raw.copy(source, _resolve_context(context), out)

def fill(
	out: Array, fill_value: Any,
	context: ExecutionContext | None = None
) -> None:
	"""Fill an array with the given value in place."""
	_raw.fill(out, fill_value, _resolve_context(context))

# -- fourier --------------------------------------------------------------

def _resolve_normalization(
	norm: str | FourierNormalization
) -> FourierNormalization:
	if isinstance(norm, FourierNormalization):
		return norm
	return FourierNormalization[norm]

def fft(
	x: Array,
	axes: Sequence[int] | None = None,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform an array to the frequency domain.

	Args:
		x: The array.
		axes: The axes to transform. Negative values count from the end.
			Defaults to the last axis.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	if axes is None:
		return _raw.fft(
			x, _resolve_normalization(norm),
			_resolve_context(context), out
		)
	return _raw.fft(
		x, list(axes), _resolve_normalization(norm),
		_resolve_context(context), out
	)

def fft2(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last two axes to the frequency domain.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.fft2(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def fft3(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last three axes to the frequency domain.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.fft3(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def ifft(
	x: Array,
	axes: Sequence[int] | None = None,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform a spectrum back to the signal domain.

	Args:
		x: The array.
		axes: The axes to transform. Negative values count from the end.
			Defaults to the last axis.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	if axes is None:
		return _raw.ifft(
			x, _resolve_normalization(norm),
			_resolve_context(context), out
		)
	return _raw.ifft(
		x, list(axes), _resolve_normalization(norm),
		_resolve_context(context), out
	)

def ifft2(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last two axes back to the signal domain.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.ifft2(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def ifft3(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last three axes back to the signal domain.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.ifft3(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def rfft(
	x: Array,
	axes: Sequence[int] | None = None,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform a real array to its half spectrum.

	Args:
		x: The array.
		axes: The axes to transform. Negative values count from the end.
			Defaults to the last axis.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	if axes is None:
		return _raw.rfft(
			x, _resolve_normalization(norm),
			_resolve_context(context), out
		)
	return _raw.rfft(
		x, list(axes), _resolve_normalization(norm),
		_resolve_context(context), out
	)

def rfft2(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last two axes of a real array to a half spectrum.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.rfft2(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def rfft3(
	x: Array,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform the last three axes of a real array to a half spectrum.

	Args:
		x: The array.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.rfft3(
		x, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def irfft(
	x: Array,
	extent: int,
	axes: Sequence[int] | None = None,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform a half spectrum back to a real array.

	Args:
		x: The array.
		extent: The number of samples along the last axis
			transformed, which its half spectrum does not tell.
		axes: The axes to transform. Negative values count from the end.
			Defaults to the last axis.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	if axes is None:
		return _raw.irfft(
			x, extent, _resolve_normalization(norm),
			_resolve_context(context), out
		)
	return _raw.irfft(
		x, list(axes), extent, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def irfft2(
	x: Array,
	extent: int,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform a half spectrum of the last two axes back to a real array.

	Args:
		x: The array.
		extent: The number of samples along the last axis, which its
			half spectrum does not tell.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.irfft2(
		x, extent, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def irfft3(
	x: Array,
	extent: int,
	norm: str | FourierNormalization = "backward",
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Transform a half spectrum of the last three axes back to a real array.

	Args:
		x: The array.
		extent: The number of samples along the last axis, which its
			half spectrum does not tell.
		norm: Which of a transform and its inverse divides by the number
			of samples: "backward", "ortho" or "forward", as in NumPy.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The transformed array.
	"""
	return _raw.irfft3(
		x, extent, _resolve_normalization(norm),
		_resolve_context(context), out
	)

def fftshift(
	x: Array,
	axes: Sequence[int] | None = None,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Move the zero frequency of a spectrum to its centre.

	Args:
		x: The array.
		axes: The axes to shift. Negative values count from the end.
			Defaults to every axis.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The shifted array.
	"""
	if axes is None:
		return _raw.fftshift(x, _resolve_context(context), out)
	return _raw.fftshift(x, list(axes), _resolve_context(context), out)

def ifftshift(
	x: Array,
	axes: Sequence[int] | None = None,
	context: ExecutionContext | None = None,
	out: Array | None = None
) -> Array:
	"""Undo fftshift, moving the centre back to index 0.

	Args:
		x: The array.
		axes: The axes to shift. Negative values count from the end.
			Defaults to every axis.
		context: The execution context. Defaults to the active one.
		out: An array to write the result into.

	Returns:
		The shifted array.
	"""
	if axes is None:
		return _raw.ifftshift(x, _resolve_context(context), out)
	return _raw.ifftshift(x, list(axes), _resolve_context(context), out)
