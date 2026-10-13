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

def test_operation_writes_into_out(__setup_context):
	x = __setup_array(__setup_context)
	y = __setup_array(__setup_context)
	out = __setup_array(__setup_context)
	result = rexlib.add(x, y, __setup_context, out=out)
	assert list(memoryview(out).cast("B").cast("f")) == [2.0] * 6
	assert list(memoryview(result).cast("B").cast("f")) == [2.0] * 6

@pytest.mark.parametrize(
	"operand",
	[None, [1.0, 2.0, 3.0], 1.0],
	ids=["none", "list", "scalar"],
)
def test_operation_refuses_an_operand_that_is_not_an_array(
	__setup_context, operand
):
	x = __setup_array(__setup_context)
	with pytest.raises(TypeError):
		rexlib.add(x, operand, __setup_context)
	with pytest.raises(TypeError):
		rexlib.negate(operand, __setup_context)

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
