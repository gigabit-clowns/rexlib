# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

@pytest.mark.parametrize(
	"operation",
	[
		rexlib.add,
		rexlib.subtract,
		rexlib.multiply,
		rexlib.divide,
		rexlib.modulo,
	]
)
def test_binary_operation_returns_array(__setup_context, operation):
	x = __setup_array(__setup_context)
	y = __setup_array(__setup_context)
	result = operation(x, y, __setup_context)
	assert isinstance(result, rexlib.Array)

@pytest.mark.parametrize(
	"operation",
	[
		rexlib.negate,
		rexlib.abs,
	]
)
def test_unary_operation_returns_array(__setup_context, operation):
	x = __setup_array(__setup_context)
	result = operation(x, __setup_context)
	assert isinstance(result, rexlib.Array)

def __setup_array(context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.float32
	)
	return rexlib.ones(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, context
	)

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	return rexlib.dispatch.ExecutionContext(device_context, dispatcher)
