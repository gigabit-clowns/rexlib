# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_empty_returns_array(__setup_context):
	result = xmipp4.functional.empty(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_zeros_returns_array(__setup_context):
	result = xmipp4.functional.zeros(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_ones_returns_array(__setup_context):
	result = xmipp4.functional.ones(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_full_returns_array(__setup_context):
	result = xmipp4.functional.full(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		3.5,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_full_supports_float16(__setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float16
	)
	result = xmipp4.functional.full(
		descriptor,
		xmipp4.hardware.MemoryResourceAffinity.host,
		1.5,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_copy_returns_array(__setup_context):
	source = xmipp4.functional.zeros(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	result = xmipp4.functional.copy(source, __setup_context)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_fill_does_not_raise(__setup_context):
	target = xmipp4.functional.empty(
		__setup_descriptor(),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	xmipp4.functional.fill(target, 7.0, __setup_context)

def __setup_descriptor():
	return xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.float32
	)

@pytest.fixture
def __setup_context():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	dispatcher = xmipp4.dispatch.make_eager_dispatcher(program_manager)
	return xmipp4.dispatch.ExecutionContext(device_context, dispatcher)
