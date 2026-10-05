# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_reports_what_a_single_image_was_written_with(__written_image):
	descriptor = image.query_descriptor(__written_image)
	assert descriptor.extents == (4, 6)
	assert descriptor.core_rank == 2
	assert descriptor.data_type == rexlib.NumericalType.float32

def test_reports_a_stack_with_the_axis_it_stacks_along_first(__written_stack):
	descriptor = image.query_descriptor(__written_stack)
	assert descriptor.extents == (3, 4, 6)
	assert descriptor.core_rank == 2

def test_returns_an_image_descriptor(__written_image):
	descriptor = image.query_descriptor(__written_image)
	assert isinstance(descriptor, image.ImageDescriptor)

def test_what_it_reports_sizes_a_destination(__written_stack, __setup_context):
	descriptor = image.query_descriptor(__written_stack)
	core = image.get_core_extents(descriptor)
	destination = rexlib.zeros(
		rexlib.make_contiguous_array_descriptor(
			(2, *core), descriptor.data_type
		),
		rexlib.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	assert destination.shape[1:] == core

def test_accepts_an_explicit_provider(__written_image):
	catalog = rexlib.ServiceCatalog()
	readers = image.DirectImageReaderProvider(
		image.get_image_read_format_manager(catalog)
	)
	assert image.query_descriptor(__written_image, readers).extents == (4, 6)

def test_a_caching_provider_keeps_the_reader_it_opened(__written_image):
	readers = image.reader_provider(cache=4)
	image.query_descriptor(__written_image, readers)
	image.query_descriptor(__written_image, readers)
	assert readers.capacity == 4
	assert readers.reader_count == 1

def test_raises_on_a_file_that_is_not_there():
	with pytest.raises(RuntimeError):
		image.query_descriptor('/path/to/no/such/file.mrc')

@pytest.fixture
def __written_image(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((4, 6), __setup_context), path)
	return path

@pytest.fixture
def __written_stack(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	image.write_stack(__setup_array((3, 4, 6), __setup_context), path)
	return path

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
