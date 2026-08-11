# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
import threading

from ._catalog import get_default_catalog
from ._core_binding import dispatch, hardware
from ._core_binding.dispatch import ExecutionContext

__local = threading.local()

def get_active_execution_context() -> ExecutionContext:
	"""
	Get the execution context active on this thread.

	Lazily initialized, on first access from a given thread, to a basal
	state: an empty `xmipp4.hardware.DeviceContext` (see its "Empty state"
	docs) paired with an eager dispatcher. `with xmipp4.device(...):` derives
	from whatever is currently active via `ExecutionContext.with_device_context`
	instead of replacing it, so this basal dispatcher is built once per
	thread and reused, rather than being rebuilt every time a
	`with xmipp4.device(...):` block is entered without an outer one already
	active.

	Returns:
		ExecutionContext: The execution context active on this thread.
	"""
	context = getattr(__local, "execution_context", None)
	if context is None:
		catalog = get_default_catalog()
		program_manager = dispatch.get_program_manager(catalog)
		dispatcher = dispatch.make_eager_dispatcher(program_manager)
		context = ExecutionContext(hardware.DeviceContext(), dispatcher)
		__local.execution_context = context
	return context

def _set_active_execution_context(
	context: ExecutionContext
) -> ExecutionContext:
	"""Set the active execution context, returning the previous one."""
	previous = get_active_execution_context()
	__local.execution_context = context
	return previous
