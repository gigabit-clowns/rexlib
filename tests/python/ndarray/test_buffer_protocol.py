# SPDX-License-Identifier: GPL-3.0-only

import gc

import numpy
import pytest

import rexlib

HOST = rexlib.hardware.MemoryResourceAffinity.host

@pytest.mark.parametrize(
	('data_type', 'dtype'),
	[
		(rexlib.NumericalType.boolean, numpy.bool_),
		(rexlib.NumericalType.int8, numpy.int8),
		(rexlib.NumericalType.uint8, numpy.uint8),
		(rexlib.NumericalType.int16, numpy.int16),
		(rexlib.NumericalType.uint16, numpy.uint16),
		(rexlib.NumericalType.int32, numpy.int32),
		(rexlib.NumericalType.uint32, numpy.uint32),
		(rexlib.NumericalType.int64, numpy.int64),
		(rexlib.NumericalType.uint64, numpy.uint64),
		(rexlib.NumericalType.float16, numpy.float16),
		(rexlib.NumericalType.float32, numpy.float32),
		(rexlib.NumericalType.float64, numpy.float64),
		(rexlib.NumericalType.complex_float32, numpy.complex64),
		(rexlib.NumericalType.complex_float64, numpy.complex128),
	]
)
def test_numpy_sees_the_data_type(data_type, dtype, __setup_context):
	array = __setup_empty([2, 3], data_type, __setup_context)
	assert numpy.asarray(array).dtype == dtype

def test_numpy_sees_a_character_as_an_8_bit_integer(__setup_context):
	array = __setup_empty(
		[2, 3], rexlib.NumericalType.char8, __setup_context
	)
	dtype = numpy.asarray(array).dtype
	assert dtype.kind in 'iu'
	assert dtype.itemsize == 1

def test_numpy_sees_the_shape(__setup_context):
	array = __setup_ones([4, 6], __setup_context)
	assert numpy.asarray(array).shape == (4, 6)

def test_numpy_sees_the_values(__setup_context):
	array = __setup_full([2, 3], 2.5, __setup_context)
	expected = numpy.full((2, 3), 2.5, dtype=numpy.float32)
	assert numpy.array_equal(numpy.asarray(array), expected)

def test_memoryview_describes_the_array(__setup_context):
	view = memoryview(__setup_ones([2, 3], __setup_context))
	assert view.format == 'f'
	assert view.itemsize == 4
	assert view.shape == (2, 3)
	assert view.strides == (12, 4)
	assert view.c_contiguous
	assert not view.readonly

def test_the_memory_is_shared_and_not_copied(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	assert numpy.shares_memory(numpy.asarray(array), numpy.asarray(array))

def test_a_write_through_numpy_is_seen_by_an_operation(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	numpy.asarray(array)[...] = 3
	result = rexlib.add(array, array, __setup_context)
	assert numpy.array_equal(
		numpy.asarray(result), numpy.full((2, 3), 6, dtype=numpy.float32)
	)

def test_a_write_by_an_operation_is_seen_through_numpy(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	view = numpy.asarray(array)
	rexlib.fill(array, 5, __setup_context)
	assert numpy.array_equal(view, numpy.full((2, 3), 5, dtype=numpy.float32))

def test_the_view_keeps_the_memory_alive(__setup_context):
	view = numpy.asarray(__setup_ones([2, 3], __setup_context))
	gc.collect()
	assert view.sum() == 6

def test_array_method_gives_a_view_by_default(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	assert numpy.shares_memory(array.__array__(), numpy.asarray(array))

def test_array_method_converts_to_the_requested_type(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	converted = array.__array__(dtype=numpy.float64)
	assert converted.dtype == numpy.float64
	assert numpy.array_equal(converted, numpy.ones((2, 3)))

def test_array_method_copies_when_asked_to(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	copied = array.__array__(copy=True)
	assert not numpy.shares_memory(copied, numpy.asarray(array))

def test_a_type_with_no_buffer_format_is_refused(__setup_context):
	array = __setup_empty(
		[2, 3], rexlib.NumericalType.complex_float16, __setup_context
	)
	with pytest.raises(BufferError):
		memoryview(array)

def test_numpy_does_not_wrap_an_array_it_cannot_view(__setup_context):
	array = __setup_empty(
		[2, 3], rexlib.NumericalType.complex_float16, __setup_context
	)
	with pytest.raises(BufferError):
		numpy.asarray(array)

def __setup_empty(shape, data_type, context):
	descriptor = rexlib.make_contiguous_array_descriptor(shape, data_type)
	return rexlib.empty(descriptor, HOST, context)

def __setup_ones(shape, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.ones(descriptor, HOST, context)

def __setup_full(shape, value, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.full(descriptor, HOST, value, context)

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	return rexlib.dispatch.ExecutionContext(device_context, dispatcher)
