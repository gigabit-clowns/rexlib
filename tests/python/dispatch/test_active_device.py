# SPDX-License-Identifier: GPL-3.0-only

import rexlib

def _is_basal(context) -> bool:
	return context.device_context.device_session is None

def test_no_active_context_by_default():
	assert _is_basal(rexlib.get_active_execution_context())

def test_device_activates_context_for_the_duration_of_the_block():
	assert _is_basal(rexlib.get_active_execution_context())
	with rexlib.device('cpu') as ctx:
		assert rexlib.get_active_execution_context() is ctx
	assert _is_basal(rexlib.get_active_execution_context())

def test_device_accepts_a_device_index():
	index = rexlib.hardware.DeviceIndex('cpu', 0)
	with rexlib.device(index) as ctx:
		assert rexlib.get_active_execution_context() is ctx

def test_device_accepts_a_device_session():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	with rexlib.device(session) as ctx:
		assert ctx.device_context.device_session is session

def test_device_accepts_a_device_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	with rexlib.device(device_context) as ctx:
		assert ctx.device_context.device_session is session

def test_nested_device_restores_the_outer_context():
	with rexlib.device('cpu') as outer:
		with rexlib.device('cpu') as inner:
			assert rexlib.get_active_execution_context() is inner
			assert inner is not outer
		assert rexlib.get_active_execution_context() is outer
	assert _is_basal(rexlib.get_active_execution_context())

def test_device_restores_previous_context_on_exception():
	with rexlib.device('cpu') as outer:
		try:
			with rexlib.device('cpu'):
				raise ValueError('boom')
		except ValueError:
			pass
		assert rexlib.get_active_execution_context() is outer

def test_nested_device_preserves_the_outer_dispatcher():
	with rexlib.device('cpu') as outer, rexlib.device('cpu') as inner:
		assert inner.dispatcher is outer.dispatcher
		assert inner.device_context is not outer.device_context

def test_basal_dispatcher_is_reused_across_unnested_device_blocks():
	with rexlib.device('cpu') as first:
		pass
	with rexlib.device('cpu') as second:
		pass
	assert first.dispatcher is second.dispatcher
