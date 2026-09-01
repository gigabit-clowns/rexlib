# SPDX-License-Identifier: GPL-3.0-only

"""Arrays, enriched with the Python operators the bound type cannot carry.

Wraps `rexlib._binding.ndarray`, adding operators to `Array`.
Operators cannot take an execution context, so they go through
`rexlib`, which falls back to the active one (see
`rexlib.get_active_execution_context`).

The operators are installed onto the bound `Array` rather than onto a
subclass: every array is built in C++, so a subclass would only ever be
seen by callers that construct one by hand.
"""

from __future__ import annotations

from . import _functional
from ._binding.ndarray import Array

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

Array.__add__ = _binary_operator(_functional.add)
Array.__sub__ = _binary_operator(_functional.subtract)
Array.__mul__ = _binary_operator(_functional.multiply)
Array.__truediv__ = _binary_operator(_functional.divide)
Array.__mod__ = _binary_operator(_functional.modulo)

Array.__iadd__ = _in_place_operator(_functional.add)
Array.__isub__ = _in_place_operator(_functional.subtract)
Array.__imul__ = _in_place_operator(_functional.multiply)
Array.__itruediv__ = _in_place_operator(_functional.divide)
Array.__imod__ = _in_place_operator(_functional.modulo)

Array.__neg__ = _unary_operator(_functional.negate)
Array.__abs__ = _unary_operator(_functional.abs)
Array.__pos__ = _unary_operator(_functional.copy)

Array.__copy__ = _unary_operator(_functional.copy)
Array.__deepcopy__ = _deep_copy
