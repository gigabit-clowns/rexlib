# SPDX-License-Identifier: GPL-3.0-only

"""Arrays, enriched with the Python operators the bound type cannot carry.

Wraps `rexlib._binding.ndarray`, adding operators to `Array`.
Operators cannot take an execution context, so they go through
`rexlib`, which falls back to the active one (see
`rexlib.get_active_execution_context`).

The operators are installed onto the bound `Array` rather than onto a
subclass: every array is built in C++, so a subclass would only ever be
seen by callers that construct one by hand.

`Array` exposes its memory through the buffer protocol, which is what
`numpy.asarray` uses. `__array__` is here for the arrays that cannot: numpy
asks it last, and would otherwise wrap such an array in an object array
instead of failing.

`__dlpack__` and `__dlpack_device__` hand the same memory to whatever
library takes DLPack, and `from_dlpack` takes the memory of whatever library
hands it out. The capsule is built and read by the binding; what the protocol
lets either side ask for is decided here.
"""

from __future__ import annotations

from . import _functional
from ._binding.ndarray import (
	Array,
	from_dlpack_capsule,
	get_dlpack_device,
	get_dlpack_version,
	is_dlpack_capsule_read_only,
	to_dlpack,
)

def _install(name: str, function) -> None:
	"""Install function onto Array as name, named as a method of Array."""
	function.__name__ = name
	function.__qualname__ = f"{Array.__qualname__}.{name}"
	function.__module__ = Array.__module__
	setattr(Array, name, function)

def _binary_operator(function):
	def operator(self: Array, other: Array) -> Array:
		if not isinstance(other, Array):
			return NotImplemented
		return function(self, other)
	return operator

def _in_place_operator(function):
	def operator(self: Array, other: Array) -> Array:
		if not isinstance(other, Array):
			return NotImplemented
		function(self, other, out=self)
		return self
	return operator

def _unary_operator(function):
	def operator(self: Array) -> Array:
		return function(self)
	return operator

def _deep_copy(self: Array, memo: dict) -> Array:
	return _functional.copy(self)

def _to_numpy(self: Array, dtype=None, copy=None):
	import numpy  # noqa: PLC0415
	return numpy.array(memoryview(self), dtype=dtype, copy=copy)

def _dlpack_device(self: Array) -> tuple[int, int]:
	return get_dlpack_device(self)

def _dlpack(
	self: Array, *,
	stream=None, max_version=None, dl_device=None, copy=None
):
	if stream is not None:
		raise ValueError(
			"A stream orders work on a device, and rexlib only exchanges "
			"host memory. Pass stream=None."
		)
	if dl_device is not None and tuple(dl_device) != _dlpack_device(self):
		raise BufferError(
			"The array is not on the requested device, and rexlib does not "
			"move it there. Transfer it first."
		)

	major = get_dlpack_version()[0]
	versioned = max_version is not None and max_version[0] >= major
	source = _functional.copy(self) if copy else self
	return to_dlpack(source, versioned, bool(copy))

def _request_dlpack(source, copy: bool | None):
	try:
		return source.__dlpack__(
			max_version=get_dlpack_version(), copy=copy
		)
	except TypeError:
		# An exporter from before DLPack 1.0 takes neither argument, so it
		# cannot be asked for a copy. It never makes one by itself.
		if copy:
			raise
		return source.__dlpack__()

def from_dlpack(source, /, *, copy: bool | None = None) -> Array:
	"""
	Create an array over the memory of an object that exports DLPack.

	The array shares the memory of `source` and keeps it alive. Only host
	memory is taken: a source on another device is refused, never moved.

	An array can always be written to, so a read-only source is not shared:
	it is copied, or refused under `copy=False`. Some libraries do not say
	that their memory is read only; theirs is shared like any other.

	Args:
		source: Any object with a `__dlpack__` method.
		copy: True asks `source` for a copy of its own. False never copies.
			None copies only a read-only source, which needs an active
			device (see `rexlib.device`).

	Returns:
		An array over the memory of `source`, or over a copy of it.

	Raises:
		BufferError: `source` is not in host memory, holds a data type no
			array has, is not aligned for its data type, or is read only
			under `copy=False`.
	"""
	capsule = _request_dlpack(source, copy)
	if not is_dlpack_capsule_read_only(capsule):
		return from_dlpack_capsule(capsule)

	if copy is False:
		raise BufferError(
			"The source is read only, and an array can always be written "
			"to. Pass copy=None to have it copied."
		)
	return _functional.copy(from_dlpack_capsule(capsule))

_install("__add__", _binary_operator(_functional.add))
_install("__sub__", _binary_operator(_functional.subtract))
_install("__mul__", _binary_operator(_functional.multiply))
_install("__truediv__", _binary_operator(_functional.divide))
_install("__mod__", _binary_operator(_functional.modulo))

_install("__iadd__", _in_place_operator(_functional.add))
_install("__isub__", _in_place_operator(_functional.subtract))
_install("__imul__", _in_place_operator(_functional.multiply))
_install("__itruediv__", _in_place_operator(_functional.divide))
_install("__imod__", _in_place_operator(_functional.modulo))

_install("__neg__", _unary_operator(_functional.negate))
_install("__abs__", _unary_operator(_functional.abs))
_install("__pos__", _unary_operator(_functional.copy))

_install("__copy__", _unary_operator(_functional.copy))
_install("__deepcopy__", _deep_copy)

_install("__array__", _to_numpy)
_install("__dlpack__", _dlpack)
_install("__dlpack_device__", _dlpack_device)
