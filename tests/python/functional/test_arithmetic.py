# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

@pytest.mark.parametrize(
	"operation",
	[
		xmipp4.functional.add,
		xmipp4.functional.subtract,
		xmipp4.functional.multiply,
		xmipp4.functional.divide,
		xmipp4.functional.modulo,
	]
)
def test_binary_operation_returns_array(__setup_context, operation):
	x = __setup_array(__setup_context)
	y = __setup_array(__setup_context)
	result = operation(x, y, __setup_context)
	assert isinstance(result, xmipp4.ndarray.Array)

@pytest.mark.parametrize(
	"operation",
	[
		xmipp4.functional.negate,
		xmipp4.functional.abs,
	]
)
def test_unary_operation_returns_array(__setup_context, operation):
	x = __setup_array(__setup_context)
	result = operation(x, __setup_context)
	assert isinstance(result, xmipp4.ndarray.Array)

def __setup_array(context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.float32
	)
	return xmipp4.functional.ones(
		descriptor, xmipp4.hardware.MemoryResourceAffinity.host, context
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
