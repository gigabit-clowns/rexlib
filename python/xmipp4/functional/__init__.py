# SPDX-License-Identifier: GPL-3.0-only

"""Array operations with an optional, implicit execution context.

Wraps `xmipp4._core_binding.functional`: same functions and parameter
order, but `context` defaults to the active one (see
`xmipp4.get_active_execution_context`) instead of being required.
"""

from __future__ import annotations
from typing import Any

from .._core_binding import functional as _raw
from .._core_binding.ndarray import Array, ArrayDescriptor
from .._core_binding.numerical import NumericalType
from .._core_binding.hardware import MemoryResourceAffinity
from .._core_binding.dispatch import ExecutionContext
from .._context import get_active_execution_context

def _resolve_context(context: ExecutionContext | None) -> ExecutionContext:
	if context is None:
		context = get_active_execution_context()
		if context.device_context.device_session is None:
			raise RuntimeError(
				"No execution context was provided and there is no active "
				"device context. Pass context= explicitly or use "
				"'with xmipp4.device(...):'."
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
