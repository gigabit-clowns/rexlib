# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
from typing import Any, Optional

from .._core_binding import functional as _raw
from .._core_binding.ndarray import Array, ArrayDescriptor
from .._core_binding.numerical import NumericalType
from .._core_binding.hardware import MemoryResourceAffinity
from .._core_binding.dispatch import ExecutionContext
from .._context import get_active_execution_context

def _resolve_context(context: Optional[ExecutionContext]) -> ExecutionContext:
	if context is None:
		context = get_active_execution_context()
		if context is None:
			raise RuntimeError(
				"No execution context was provided and there is no active "
				"device context. Pass context= explicitly or use "
				"'with xmipp4.device.backend(...):'."
			)
	return context

# -- arithmetic -----------------------------------------------------------

def add(
	x: Array, y: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.add(x, y, _resolve_context(context), out)

def subtract(
	x: Array, y: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.subtract(x, y, _resolve_context(context), out)

def negate(
	x: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.negate(x, _resolve_context(context), out)

def abs(
	x: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.abs(x, _resolve_context(context), out)

def multiply(
	x: Array, y: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.multiply(x, y, _resolve_context(context), out)

def divide(
	x: Array, y: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.divide(x, y, _resolve_context(context), out)

def modulo(
	x: Array, y: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.modulo(x, y, _resolve_context(context), out)

# -- cast -------------------------------------------------------------------

def cast(
	input: Array, target_type: NumericalType,
	context: Optional[ExecutionContext] = None
) -> Array:
	return _raw.cast(input, target_type, _resolve_context(context))

def cast_copy(
	input: Array, target_type: NumericalType,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.cast_copy(input, target_type, _resolve_context(context), out)

# -- transfer -----------------------------------------------------------

def transfer(
	input: Array, affinity: MemoryResourceAffinity,
	context: Optional[ExecutionContext] = None
) -> Array:
	return _raw.transfer(input, affinity, _resolve_context(context))

def transfer_copy(
	input: Array, affinity: MemoryResourceAffinity,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.transfer_copy(input, affinity, _resolve_context(context), out)

def to_device(
	input: Array,
	context: Optional[ExecutionContext] = None
) -> Array:
	return _raw.to_device(input, _resolve_context(context))

def to_device_copy(
	input: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.to_device_copy(input, _resolve_context(context), out)

def to_host(
	input: Array,
	context: Optional[ExecutionContext] = None
) -> Array:
	return _raw.to_host(input, _resolve_context(context))

def to_host_copy(
	input: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.to_host_copy(input, _resolve_context(context), out)

# -- creation -----------------------------------------------------------

def empty(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.empty(descriptor, affinity, _resolve_context(context), out)

def zeros(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.zeros(descriptor, affinity, _resolve_context(context), out)

def ones(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.ones(descriptor, affinity, _resolve_context(context), out)

def full(
	descriptor: ArrayDescriptor, affinity: MemoryResourceAffinity, fill_value: Any,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.full(descriptor, affinity, fill_value, _resolve_context(context), out)

def copy(
	source: Array,
	context: Optional[ExecutionContext] = None,
	out: Optional[Array] = None
) -> Array:
	return _raw.copy(source, _resolve_context(context), out)

def fill(
	out: Array, fill_value: Any,
	context: Optional[ExecutionContext] = None
) -> None:
	_raw.fill(out, fill_value, _resolve_context(context))
