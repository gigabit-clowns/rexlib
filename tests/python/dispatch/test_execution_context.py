# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_empty_context_has_no_dispatcher():
	ctx = rexlib.dispatch.ExecutionContext()
	assert ctx.dispatcher is None

def test_context_wraps_device_context_and_dispatcher(__setup_context):
	device_context, dispatcher, ctx = __setup_context
	assert ctx.device_context.device_session is device_context.device_session
	assert ctx.dispatcher is dispatcher

def test_eager_dispatcher_is_a_dispatcher():
	catalog = rexlib.ServiceCatalog()
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	assert isinstance(program_manager, rexlib.dispatch.ProgramManager)
	assert isinstance(
		rexlib.dispatch.make_eager_dispatcher(program_manager),
		rexlib.dispatch.Dispatcher
	)

def test_context_usable_for_array_creation(__setup_context):
	_, _, ctx = __setup_context
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	array = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, ctx
	)
	assert isinstance(array, rexlib.Array)

def test_with_dispatcher_returns_updated_copy(__setup_context):
	device_context, dispatcher, ctx = __setup_context
	catalog = rexlib.ServiceCatalog()
	other = rexlib.dispatch.make_eager_dispatcher(
		rexlib.dispatch.get_program_manager(catalog)
	)
	updated = ctx.with_dispatcher(other)
	assert updated.dispatcher is other
	assert updated.device_context.device_session is device_context.device_session
	assert ctx.dispatcher is dispatcher

def test_with_device_context_returns_updated_copy(__setup_context):
	_, dispatcher, ctx = __setup_context
	empty = rexlib.hardware.DeviceContext()
	updated = ctx.with_device_context(empty)
	assert updated.device_context.device_session is None
	assert updated.dispatcher is dispatcher
	assert ctx.device_context.device_session is not None

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	ctx = rexlib.dispatch.ExecutionContext(device_context, dispatcher)
	return device_context, dispatcher, ctx
