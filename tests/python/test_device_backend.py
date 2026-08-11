# SPDX-License-Identifier: GPL-3.0-only

import xmipp4

def _is_basal(context) -> bool:
	return context.device_context.device_session is None

def test_no_active_context_by_default():
	assert _is_basal(xmipp4.get_active_execution_context())

def test_device_activates_context_for_the_duration_of_the_block():
	assert _is_basal(xmipp4.get_active_execution_context())
	with xmipp4.device('cpu') as ctx:
		assert xmipp4.get_active_execution_context() is ctx
	assert _is_basal(xmipp4.get_active_execution_context())

def test_device_accepts_a_device_index():
	index = xmipp4.hardware.DeviceIndex('cpu', 0)
	with xmipp4.device(index) as ctx:
		assert xmipp4.get_active_execution_context() is ctx

def test_device_accepts_a_device_session():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	with xmipp4.device(session) as ctx:
		assert ctx.device_context.device_session is session

def test_device_accepts_a_device_context():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	with xmipp4.device(device_context) as ctx:
		assert ctx.device_context.device_session is session

def test_nested_device_restores_the_outer_context():
	with xmipp4.device('cpu') as outer:
		with xmipp4.device('cpu') as inner:
			assert xmipp4.get_active_execution_context() is inner
			assert inner is not outer
		assert xmipp4.get_active_execution_context() is outer
	assert _is_basal(xmipp4.get_active_execution_context())

def test_device_restores_previous_context_on_exception():
	with xmipp4.device('cpu') as outer:
		try:
			with xmipp4.device('cpu'):
				raise ValueError('boom')
		except ValueError:
			pass
		assert xmipp4.get_active_execution_context() is outer

def test_nested_device_preserves_the_outer_dispatcher():
	with xmipp4.device('cpu') as outer, xmipp4.device('cpu') as inner:
		assert inner.dispatcher is outer.dispatcher
		assert inner.device_context is not outer.device_context

def test_basal_dispatcher_is_reused_across_unnested_device_blocks():
	with xmipp4.device('cpu') as first:
		pass
	with xmipp4.device('cpu') as second:
		pass
	assert first.dispatcher is second.dispatcher
