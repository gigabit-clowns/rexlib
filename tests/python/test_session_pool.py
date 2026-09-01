# SPDX-License-Identifier: GPL-3.0-only

import threading

import rexlib

def test_same_device_reuses_the_session():
	with rexlib.device('cpu') as first, rexlib.device('cpu') as second:
		assert (
			first.device_context.device_session
			is second.device_context.device_session
		)

def test_session_survives_leaving_the_block():
	with rexlib.device('cpu') as first:
		session = first.device_context.device_session
	with rexlib.device('cpu') as second:
		assert second.device_context.device_session is session

def test_device_index_and_string_share_the_session():
	with rexlib.device('cpu') as from_string:
		session = from_string.device_context.device_session
	with rexlib.device(rexlib.hardware.DeviceIndex('cpu', 0)) as from_index:
		assert from_index.device_context.device_session is session

def test_explicit_session_bypasses_the_pool():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	own = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	with rexlib.device(own) as ctx:
		assert ctx.device_context.device_session is own
	with rexlib.device('cpu') as pooled:
		assert pooled.device_context.device_session is not own

def test_concurrent_activation_shares_one_session():
	sessions = []
	barrier = threading.Barrier(8)

	def activate():
		barrier.wait()
		with rexlib.device('cpu') as ctx:
			sessions.append(ctx.device_context.device_session)

	threads = [threading.Thread(target=activate) for _ in range(8)]
	for thread in threads:
		thread.start()
	for thread in threads:
		thread.join()

	assert len(sessions) == 8
	assert all(session is sessions[0] for session in sessions)
