# SPDX-License-Identifier: GPL-3.0-only

import xmipp4

def test_no_active_context_by_default():
	assert xmipp4.get_active_execution_context() is None

def test_backend_activates_context_for_the_duration_of_the_block():
	assert xmipp4.get_active_execution_context() is None
	with xmipp4.device.backend('cpu') as ctx:
		assert xmipp4.get_active_execution_context() is ctx
	assert xmipp4.get_active_execution_context() is None

def test_backend_accepts_a_device_index():
	index = xmipp4.hardware.DeviceIndex('cpu', 0)
	with xmipp4.device.backend(index) as ctx:
		assert xmipp4.get_active_execution_context() is ctx

def test_nested_backend_restores_the_outer_context():
	with xmipp4.device.backend('cpu') as outer:
		with xmipp4.device.backend('cpu') as inner:
			assert xmipp4.get_active_execution_context() is inner
			assert inner is not outer
		assert xmipp4.get_active_execution_context() is outer
	assert xmipp4.get_active_execution_context() is None

def test_backend_restores_previous_context_on_exception():
	with xmipp4.device.backend('cpu') as outer:
		try:
			with xmipp4.device.backend('cpu'):
				raise ValueError('boom')
		except ValueError:
			pass
		assert xmipp4.get_active_execution_context() is outer
