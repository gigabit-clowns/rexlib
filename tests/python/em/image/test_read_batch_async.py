# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_reads_a_batch(__written_files, __setup_context):
	loader = image.loader()
	destination = __setup_destination(__written_files, __setup_context)
	image.read_batch_async(loader, destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_the_destination_survives_the_read(__written_files, __setup_context):
	# The read takes the array by value and array is move-only, so a
	# binding that moved rather than shared would leave this one empty.
	loader = image.loader()
	destination = __setup_destination(__written_files, __setup_context)
	image.read_batch_async(loader, destination, __written_files).get()
	assert destination.shape == (len(__written_files), 4, 6)

def test_batches_in_flight_can_be_collected_together(
	__written_files, __setup_context
):
	loader = image.loader()
	first = __setup_destination(__written_files, __setup_context)
	second = __setup_destination(__written_files, __setup_context)
	completions = [
		image.read_batch_async(loader, first, __written_files),
		image.read_batch_async(loader, second, __written_files),
	]
	for completion in completions:
		completion.get()
	assert all(c.is_ready for c in completions)

def test_a_completion_reports_when_it_is_done(__written_files, __setup_context):
	loader = image.loader()
	destination = __setup_destination(__written_files, __setup_context)
	completion = image.read_batch_async(loader, destination, __written_files)
	completion.wait()
	assert completion.is_ready

def test_an_empty_batch_is_already_done(__setup_context):
	loader = image.loader()
	destination = __setup_array((0, 4, 6), __setup_context)
	assert image.read_batch_async(loader, destination, []).is_ready

def test_a_destination_of_the_wrong_batch_size_is_refused(
	__written_files, __setup_context
):
	loader = image.loader()
	destination = __setup_array(
		(len(__written_files) + 1, 4, 6), __setup_context
	)
	with pytest.raises(ValueError):
		image.read_batch_async(loader, destination, __written_files)

def test_reads_the_images_of_a_stack_by_their_index(
	__written_stack, __setup_context
):
	loader = image.loader()
	locations = [
		image.ImageLocation(__written_stack, 2),
		image.ImageLocation(__written_stack, 0),
	]
	destination = __setup_array((2, 4, 6), __setup_context)
	image.read_batch_async(loader, destination, locations).get()
	assert destination.shape == (2, 4, 6)

def test_an_index_past_the_end_of_a_stack_is_reported(
	__written_stack, __setup_context
):
	loader = image.loader()
	locations = [
		image.ImageLocation(__written_stack, 0),
		image.ImageLocation(__written_stack, 3),
	]
	destination = __setup_array((2, 4, 6), __setup_context)
	completion = image.read_batch_async(loader, destination, locations)
	with pytest.raises(IndexError):
		completion.get()

def test_locations_with_and_without_an_index_do_not_mix(
	__written_stack, __setup_context
):
	loader = image.loader()
	locations = [
		image.ImageLocation(__written_stack, 0),
		image.ImageLocation(__written_stack),
	]
	destination = __setup_array((2, 4, 6), __setup_context)
	with pytest.raises(ValueError):
		image.read_batch_async(loader, destination, locations)

def test_runs_on_a_synchronous_executor_too(__written_files, __setup_context):
	loader = image.loader(executor=rexlib.concurrency.SynchronousExecutor())
	destination = __setup_destination(__written_files, __setup_context)
	image.read_batch_async(loader, destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_a_caching_provider_serves_repeated_reads(
	__written_files, __setup_context
):
	loader = image.loader(cache=4)
	destination = __setup_destination(__written_files, __setup_context)
	image.read_batch_async(loader, destination, __written_files).get()
	image.read_batch_async(loader, destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_loader_returns_an_image_loader():
	loader = image.loader(workers=2)
	assert isinstance(loader, image.ExecutorImageLoader)
	assert isinstance(loader, image.ImageLoader)

def test_assembled_by_hand(__written_files, __setup_context):
	catalog = rexlib.ServiceCatalog()
	readers = image.DirectImageReaderProvider(
		image.get_image_read_format_manager(catalog)
	)
	executor = rexlib.concurrency.ThreadPoolExecutor(2)
	loader = image.ExecutorImageLoader(readers, executor)
	destination = __setup_destination(__written_files, __setup_context)
	image.read_batch_async(loader, destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def __setup_destination(locations, context):
	descriptor = image.query_descriptor(locations[0].path)
	return __setup_array(
		(len(locations), *image.get_core_extents(descriptor)), context
	)

def __setup_array(shape, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, context
	)

@pytest.fixture
def __written_files(tmp_path, __setup_context):
	source = __setup_array((4, 6), __setup_context)
	locations = []
	for index in range(3):
		path = str(tmp_path / f'image{index}.mrc')
		image.write_single(source, path)
		locations.append(image.ImageLocation(path))
	return locations

@pytest.fixture
def __written_stack(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	image.write_stack(__setup_array((3, 4, 6), __setup_context), path)
	return path

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	return rexlib.dispatch.ExecutionContext(device_context, dispatcher)
