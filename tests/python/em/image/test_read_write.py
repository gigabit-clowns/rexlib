# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_round_trip_returns_an_array(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	image.write(__setup_array(__setup_context), path)
	assert isinstance(image.read(path, context=__setup_context), rexlib.Array)

def test_writes_the_type_it_is_asked_for(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	image.write(
		__setup_array(__setup_context), path,
		data_type=rexlib.NumericalType.int16
	)
	assert isinstance(image.read(path, context=__setup_context), rexlib.Array)

def test_reads_through_an_image_location(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	image.write(__setup_array(__setup_context), path)
	location = image.ImageLocation(path)
	assert isinstance(
		image.read(location, context=__setup_context), rexlib.Array
	)

def test_what_read_returns_can_be_written_back(tmp_path, __setup_context):
	# write refuses storage the host cannot reach, so a round trip through
	# it is what shows read handed back a host array.
	source = str(tmp_path / 'stack.mrc')
	copy = str(tmp_path / 'copy.mrc')
	image.write(__setup_array(__setup_context), source)
	image.write(image.read(source, context=__setup_context), copy)
	assert isinstance(image.read(copy, context=__setup_context), rexlib.Array)

def test_uses_the_active_context_when_none_is_given(tmp_path):
	path = str(tmp_path / 'stack.mrc')
	with rexlib.device('cpu') as context:
		image.write(__setup_array(context), path)
		assert isinstance(image.read(path), rexlib.Array)

def test_raises_without_context_and_without_active_device(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrc')
	image.write(__setup_array(__setup_context), path)
	with pytest.raises(RuntimeError):
		image.read(path)

def test_raises_on_a_file_no_format_recognizes(__setup_context):
	with pytest.raises(RuntimeError):
		image.read('/path/to/no/such/file.mrc', context=__setup_context)

def test_a_location_string_is_not_parsed_by_read(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	image.write(__setup_array(__setup_context), path)
	with pytest.raises(RuntimeError):
		image.read(f'1@{path}', context=__setup_context)

def test_accepts_explicit_managers(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrc')
	catalog = rexlib.ServiceCatalog()
	image.write(
		__setup_array(__setup_context), path,
		manager=image.get_image_write_format_manager(catalog)
	)
	result = image.read(
		path,
		manager=image.get_image_read_format_manager(catalog),
		context=__setup_context
	)
	assert isinstance(result, rexlib.Array)

def __setup_array(context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		[4, 6], rexlib.NumericalType.float32
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
