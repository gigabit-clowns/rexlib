# SPDX-License-Identifier: GPL-3.0-only

from __future__ import annotations
import threading

from ._catalog import get_default_catalog
from ._binding import hardware

# DeviceIndex defines __eq__ without __hash__, so it cannot be a dict key.
__pool: dict[tuple[str, int], hardware.DeviceSession] = {}
__pool_lock = threading.Lock()
__index_locks: dict[tuple[str, int], threading.Lock] = {}

def get_pooled_device_session(
	index: hardware.DeviceIndex
) -> hardware.DeviceSession:
	"""
	Get the process-wide device session for a device, creating it once.

	`DeviceManager.create_device_session` is documented as uncached, so
	every call builds a new session. Sessions are expensive and holding
	several for the same device means sharing resources across distinct
	logical device handles, so this pool keeps one session per device and
	hands it out to every caller.

	Sessions are never evicted: a process that has touched a device is
	expected to keep using it until it exits.

	Creation is serialized per device, so concurrent callers asking for
	the same device get the same session, while callers asking for
	different devices never wait on each other.

	Args:
		index: The device whose session is requested.

	Returns:
		DeviceSession: The pooled session for the device.
	"""
	key = (index.backend, index.id)

	with __pool_lock:
		session = __pool.get(key)
		if session is not None:
			return session
		index_lock = __index_locks.setdefault(key, threading.Lock())

	with index_lock:
		with __pool_lock:
			session = __pool.get(key)
		if session is None:
			manager = hardware.get_device_manager(get_default_catalog())
			session = manager.create_device_session(index)
			with __pool_lock:
				__pool[key] = session
		return session
