# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
from types import TracebackType
from typing import Optional, Type, Union

from ._core_binding import hardware, dispatch
from ._catalog import get_default_catalog
from ._context import _set_active_execution_context

class _ActiveDeviceContext:
	"""Context manager that activates an ExecutionContext for its duration."""

	def __init__(self, spec: Union[str, hardware.DeviceIndex]) -> None:
		self.__spec = spec
		self.__previous: Optional[dispatch.ExecutionContext] = None

	def __enter__(self) -> dispatch.ExecutionContext:
		index = (
			hardware.DeviceIndex(self.__spec)
			if isinstance(self.__spec, str)
			else self.__spec
		)

		catalog = get_default_catalog()
		session = hardware.get_device_manager(catalog).create_device_session(index)
		device_context = hardware.DeviceContext(session)
		program_manager = dispatch.get_program_manager(catalog)
		context_dispatcher = dispatch.make_eager_dispatcher(program_manager)
		context = dispatch.ExecutionContext(device_context, context_dispatcher)

		self.__previous = _set_active_execution_context(context)
		return context

	def __exit__(
		self,
		exc_type: Optional[Type[BaseException]],
		exc_value: Optional[BaseException],
		traceback: Optional[TracebackType],
	) -> None:
		_set_active_execution_context(self.__previous)

class Device:
	"""
	Enriches xmipp4's hardware devices with ergonomics not provided by the
	raw binding, such as activating an execution context for a `with` block.
	"""

	def backend(
		self, spec: Union[str, hardware.DeviceIndex]
	) -> _ActiveDeviceContext:
		"""
		Activate a device as the current thread's execution context.

		Builds a device session, device context, and dispatcher for the
		given device, and makes the resulting execution context the active
		one (see `xmipp4.get_active_execution_context`) for the duration of
		the `with` block, restoring the previously active one on exit.

		Args:
			spec: The device to activate, either a "backend:id" /
				"backend" string (see `xmipp4.hardware.DeviceIndex`) or an
				already constructed DeviceIndex.

		Returns:
			A context manager yielding the activated ExecutionContext.
		"""
		return _ActiveDeviceContext(spec)

device = Device()
