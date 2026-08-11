# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_raises_without_context_and_without_active_device():
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with pytest.raises(RuntimeError):
		xmipp4.functional.zeros(descriptor, xmipp4.hardware.MemoryResourceAffinity.host)

def test_uses_active_context_when_none_given():
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with xmipp4.device('cpu'):
		result = xmipp4.functional.zeros(
			descriptor, xmipp4.hardware.MemoryResourceAffinity.host
		)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_explicit_context_still_works_outside_a_with_block():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	dispatcher = xmipp4.dispatch.make_eager_dispatcher(program_manager)
	context = xmipp4.dispatch.ExecutionContext(device_context, dispatcher)

	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	result = xmipp4.functional.zeros(
		descriptor, xmipp4.hardware.MemoryResourceAffinity.host, context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_explicit_context_overrides_active_context():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	dispatcher = xmipp4.dispatch.make_eager_dispatcher(program_manager)
	explicit_context = xmipp4.dispatch.ExecutionContext(device_context, dispatcher)

	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with xmipp4.device('cpu'):
		result = xmipp4.functional.zeros(
			descriptor, xmipp4.hardware.MemoryResourceAffinity.host, explicit_context
		)
	assert isinstance(result, xmipp4.ndarray.Array)
