# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_empty_context_has_no_dispatcher():
	ctx = xmipp4.dispatch.ExecutionContext()
	assert ctx.dispatcher is None

def test_context_wraps_device_context_and_dispatcher(__setup_context):
	device_context, dispatcher, ctx = __setup_context
	assert ctx.device_context.device_session is device_context.device_session
	assert ctx.dispatcher is dispatcher

def test_eager_dispatcher_is_a_dispatcher():
	catalog = xmipp4.ServiceCatalog()
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	assert isinstance(program_manager, xmipp4.dispatch.ProgramManager)
	assert isinstance(
		xmipp4.dispatch.make_eager_dispatcher(program_manager),
		xmipp4.dispatch.Dispatcher
	)

def test_context_usable_for_array_creation(__setup_context):
	_, _, ctx = __setup_context
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	array = xmipp4.functional.zeros(
		descriptor, xmipp4.hardware.MemoryResourceAffinity.host, ctx
	)
	assert isinstance(array, xmipp4.ndarray.Array)

def test_with_dispatcher_returns_updated_copy(__setup_context):
	device_context, dispatcher, ctx = __setup_context
	catalog = xmipp4.ServiceCatalog()
	other = xmipp4.dispatch.make_eager_dispatcher(
		xmipp4.dispatch.get_program_manager(catalog)
	)
	updated = ctx.with_dispatcher(other)
	assert updated.dispatcher is other
	assert updated.device_context.device_session is device_context.device_session
	assert ctx.dispatcher is dispatcher

def test_with_device_context_returns_updated_copy(__setup_context):
	_, dispatcher, ctx = __setup_context
	empty = xmipp4.hardware.DeviceContext()
	updated = ctx.with_device_context(empty)
	assert updated.device_context.device_session is None
	assert updated.dispatcher is dispatcher
	assert ctx.device_context.device_session is not None

@pytest.fixture
def __setup_context():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	dispatcher = xmipp4.dispatch.make_eager_dispatcher(program_manager)
	ctx = xmipp4.dispatch.ExecutionContext(device_context, dispatcher)
	return device_context, dispatcher, ctx
