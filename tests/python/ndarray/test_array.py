# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_reports_its_shape(__setup_context):
	array = __setup_array([4, 6], __setup_context)
	assert array.shape == (4, 6)

def test_shape_is_a_tuple(__setup_context):
	assert isinstance(__setup_array([4, 6], __setup_context).shape, tuple)

def test_reports_its_data_type(__setup_context):
	array = __setup_array([4, 6], __setup_context)
	assert array.data_type == rexlib.NumericalType.float32

def test_carries_the_descriptor_it_was_made_with(__setup_context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		[4, 6], rexlib.NumericalType.float32
	)
	array = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, __setup_context
	)
	assert array.descriptor == descriptor

def test_len_is_the_leading_extent(__setup_context):
	array = __setup_array([4, 6], __setup_context)
	assert len(array) == 4

def test_repr_names_the_type(__setup_context):
	array = __setup_array([4, 6], __setup_context)
	assert repr(array).startswith('Array(')

def test_shape_survives_the_descriptor_going_out_of_scope(__setup_context):
	# The descriptor is handed out by reference into the array, so the array
	# has to outlive it rather than the other way round.
	descriptor = __setup_array([2, 3], __setup_context).descriptor
	assert descriptor.shape == (2, 3)

def test_an_operation_keeps_the_shape(__setup_context):
	x = __setup_array([4, 6], __setup_context)
	y = __setup_array([4, 6], __setup_context)
	assert rexlib.add(x, y, __setup_context).shape == (4, 6)

def __setup_array(shape, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.zeros(
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
