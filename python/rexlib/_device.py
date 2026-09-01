# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations

from types import TracebackType
from typing import Union

from ._context import _set_active_execution_context, get_active_execution_context
from ._binding import dispatch, hardware
from ._session_pool import get_pooled_device_session

DeviceSpec = Union[
	str, hardware.DeviceIndex, hardware.DeviceSession, hardware.DeviceContext
]

def _resolve_device_context(spec: DeviceSpec) -> hardware.DeviceContext:
	if isinstance(spec, hardware.DeviceContext):
		return spec
	if isinstance(spec, hardware.DeviceSession):
		return hardware.DeviceContext(spec)

	index = (
		hardware.DeviceIndex(spec)
		if isinstance(spec, str)
		else spec
	)
	return hardware.DeviceContext(get_pooled_device_session(index))

class _ActiveDeviceContext:
	"""Context manager that activates an ExecutionContext for its duration."""

	def __init__(self, spec: DeviceSpec) -> None:
		self.__spec = spec
		self.__previous: dispatch.ExecutionContext | None = None

	def __enter__(self) -> dispatch.ExecutionContext:
		device_context = _resolve_device_context(self.__spec)
		context = get_active_execution_context().with_device_context(device_context)
		self.__previous = _set_active_execution_context(context)
		return context

	def __exit__(
		self,
		exc_type: type[BaseException] | None,
		exc_value: BaseException | None,
		traceback: TracebackType | None,
	) -> None:
		_set_active_execution_context(self.__previous)

def device(spec: DeviceSpec) -> _ActiveDeviceContext:
	"""
	Activate a device as the current thread's execution context.

	Derives an execution context from whatever is currently active (see
	`rexlib.get_active_execution_context`) by replacing its device context,
	and makes it active for the duration of the `with` block, restoring the
	previously active one on exit. Everything but the device context (e.g.
	the dispatcher) is preserved.

	Args:
		spec: The device to activate. One of:
			- A "backend:id" / "backend" string, or a `DeviceIndex`
			  (see `rexlib.hardware.DeviceIndex`): its `DeviceSession` is
			  taken from a process-wide pool, so repeatedly activating the
			  same device reuses one session instead of building a new one.
			- A `DeviceSession` (see `rexlib.hardware.DeviceSession`),
			  reused as-is, bypassing the pool.
			- A `DeviceContext` (see `rexlib.hardware.DeviceContext`),
			  used as-is.

	Returns:
		A context manager yielding the activated ExecutionContext.
	"""
	return _ActiveDeviceContext(spec)
