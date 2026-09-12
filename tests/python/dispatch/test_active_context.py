# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_raises_without_context_and_without_active_device():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	with pytest.raises(RuntimeError):
		rexlib.zeros(descriptor, rexlib.hardware.MemoryResourceAffinity.host)

def test_uses_active_context_when_none_given():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	with rexlib.device('cpu'):
		result = rexlib.zeros(
			descriptor, rexlib.hardware.MemoryResourceAffinity.host
		)
	assert isinstance(result, rexlib.Array)

def test_explicit_context_still_works_outside_a_with_block():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	context = rexlib.dispatch.ExecutionContext(device_context, dispatcher)

	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	result = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, context
	)
	assert isinstance(result, rexlib.Array)

def test_explicit_context_overrides_active_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	explicit_context = rexlib.dispatch.ExecutionContext(device_context, dispatcher)

	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	with rexlib.device('cpu'):
		result = rexlib.zeros(
			descriptor, rexlib.hardware.MemoryResourceAffinity.host, explicit_context
		)
	assert isinstance(result, rexlib.Array)
