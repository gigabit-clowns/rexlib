# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_round_trip_returns_an_array(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	assert isinstance(image.read(path, context=__setup_context), rexlib.Array)

def test_a_single_image_reads_back_with_its_shape(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	assert image.read(path, context=__setup_context).shape == (4, 6)

def test_an_array_written_single_is_one_volume(tmp_path, __setup_context):
	path = str(tmp_path / 'volume.mrc')
	image.write_single(__setup_array((3, 4, 6), __setup_context), path)
	assert image.query_descriptor(path).core_rank == 3

def test_an_array_written_as_a_stack_is_a_stack(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	image.write_stack(__setup_array((3, 4, 6), __setup_context), path)
	descriptor = image.query_descriptor(path)
	assert descriptor.extents == (3, 4, 6)
	assert descriptor.core_rank == 2

def test_a_stack_needs_an_axis_to_stack_along(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	array = __setup_array((6,), __setup_context)
	with pytest.raises(ValueError):
		image.write_stack(array, path)

def test_writes_the_type_it_is_asked_for(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(
		__setup_array((4, 6), __setup_context), path,
		data_type=rexlib.NumericalType.int16
	)
	descriptor = image.query_descriptor(path)
	assert descriptor.data_type == rexlib.NumericalType.int16

def test_writes_what_a_descriptor_states(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	descriptor = image.ImageDescriptor(
		(3, 4, 6), 2, rexlib.NumericalType.int16
	)
	image.write(__setup_array((3, 4, 6), __setup_context), path, descriptor)
	assert image.query_descriptor(path) == descriptor

def test_a_descriptor_of_another_shape_is_refused(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	descriptor = image.ImageDescriptor(
		(2, 4, 6), 2, rexlib.NumericalType.float32
	)
	array = __setup_array((3, 4, 6), __setup_context)
	with pytest.raises(ValueError):
		image.write(array, path, descriptor)

def test_reads_the_type_the_file_holds(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(
		__setup_array((4, 6), __setup_context), path,
		data_type=rexlib.NumericalType.int16
	)
	result = image.read(path, context=__setup_context)
	assert result.data_type == rexlib.NumericalType.int16

def test_reads_the_type_it_is_asked_for(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(
		__setup_array((4, 6), __setup_context), path,
		data_type=rexlib.NumericalType.int16
	)
	result = image.read(
		path, context=__setup_context,
		data_type=rexlib.NumericalType.float32
	)
	assert result.data_type == rexlib.NumericalType.float32

def test_reads_a_whole_file_through_an_image_location(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrcs')
	image.write_stack(__setup_array((3, 4, 6), __setup_context), path)
	location = image.ImageLocation(path)
	assert image.read(location, context=__setup_context).shape == (3, 4, 6)

def test_reads_one_image_of_a_stack_through_an_image_location(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrcs')
	image.write_stack(__setup_array((3, 4, 6), __setup_context), path)
	location = image.ImageLocation(path, 1)
	assert image.read(location, context=__setup_context).shape == (4, 6)

def test_what_read_returns_can_be_written_back(tmp_path, __setup_context):
	# The writes refuse storage the host cannot reach, so a round trip
	# through one is what shows read handed back a host array.
	source = str(tmp_path / 'image.mrc')
	copy = str(tmp_path / 'copy.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), source)
	image.write_single(image.read(source, context=__setup_context), copy)
	assert image.read(copy, context=__setup_context).shape == (4, 6)

def test_uses_the_active_context_when_none_is_given(tmp_path):
	path = str(tmp_path / 'image.mrc')
	with rexlib.device('cpu') as context:
		image.write_single(__setup_array((4, 6), context), path)
		assert isinstance(image.read(path), rexlib.Array)

def test_raises_without_context_and_without_active_device(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	with pytest.raises(RuntimeError):
		image.read(path)

def test_raises_on_a_file_that_is_not_there(__setup_context):
	with pytest.raises(RuntimeError):
		image.read('/path/to/no/such/file.mrc', context=__setup_context)

def test_a_location_string_is_not_parsed_by_read(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	with pytest.raises(RuntimeError):
		image.read(f'1@{path}', context=__setup_context)

def test_accepts_an_explicit_manager_and_provider(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	catalog = rexlib.ServiceCatalog()
	image.write_single(
		__setup_array((4, 6), __setup_context), path,
		manager=image.get_image_write_format_manager(catalog)
	)
	readers = image.DirectImageReaderProvider(
		image.get_image_read_format_manager(catalog)
	)
	result = image.read(path, readers=readers, context=__setup_context)
	assert isinstance(result, rexlib.Array)

def test_a_provider_assembled_with_a_cache_serves_reads(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	readers = image.reader_provider(cache=2)
	image.read(path, readers=readers, context=__setup_context)
	image.read(path, readers=readers, context=__setup_context)
	assert readers.reader_count == 1

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
