# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
from typing import Optional
import threading

from ._core_binding.dispatch import ExecutionContext

__local = threading.local()

def get_active_execution_context() -> Optional[ExecutionContext]:
	"""
	Get the execution context currently active on this thread.

	The active execution context is set by entering a
	`with xmipp4.device.backend(...):` block, and is used as the default
	`context` argument by `xmipp4.functional` when none is given explicitly.
	It is thread-local: each thread has its own, defaulting to None.

	Returns:
		Optional[ExecutionContext]: The active execution context, or None
		if none is active on this thread.
	"""
	return getattr(__local, "execution_context", None)

def _set_active_execution_context(
	context: Optional[ExecutionContext]
) -> Optional[ExecutionContext]:
	"""Set the active execution context, returning the previous one."""
	previous = get_active_execution_context()
	__local.execution_context = context
	return previous
