# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

def test_reads_a_batch(__written_files, __setup_context):
	source = image.batch_source()
	destination = __setup_destination(__written_files, __setup_context)
	source.read(destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_the_destination_survives_the_read(__written_files, __setup_context):
	# read takes the array by value and array is move-only, so a binding
	# that moved rather than shared would leave this one empty.
	source = image.batch_source()
	destination = __setup_destination(__written_files, __setup_context)
	source.read(destination, __written_files).get()
	assert destination.shape == (len(__written_files), 4, 6)

def test_batches_in_flight_can_be_collected_together(
	__written_files, __setup_context
):
	source = image.batch_source()
	first = __setup_destination(__written_files, __setup_context)
	second = __setup_destination(__written_files, __setup_context)
	completions = [
		source.read(first, __written_files),
		source.read(second, __written_files),
	]
	for completion in completions:
		completion.get()
	assert all(c.is_ready for c in completions)

def test_a_completion_reports_when_it_is_done(__written_files, __setup_context):
	source = image.batch_source()
	destination = __setup_destination(__written_files, __setup_context)
	completion = source.read(destination, __written_files)
	completion.wait()
	assert completion.is_ready

def test_an_empty_batch_is_already_done(__setup_context):
	source = image.batch_source()
	descriptor = rexlib.make_contiguous_array_descriptor(
		(0, 4, 6), rexlib.NumericalType.float32
	)
	destination = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, __setup_context
	)
	assert source.read(destination, []).is_ready

def test_a_destination_of_the_wrong_batch_size_is_refused(
	__written_files, __setup_context
):
	source = image.batch_source()
	descriptor = rexlib.make_contiguous_array_descriptor(
		(len(__written_files) + 1, 4, 6), rexlib.NumericalType.float32
	)
	destination = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, __setup_context
	)
	with pytest.raises(ValueError):
		source.read(destination, __written_files)

def test_runs_on_a_synchronous_executor_too(__written_files, __setup_context):
	source = image.batch_source(
		executor=rexlib.concurrency.SynchronousExecutor()
	)
	destination = __setup_destination(__written_files, __setup_context)
	source.read(destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_a_caching_provider_serves_repeated_reads(
	__written_files, __setup_context
):
	source = image.batch_source(cache=4)
	destination = __setup_destination(__written_files, __setup_context)
	source.read(destination, __written_files).get()
	source.read(destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def test_assembled_by_hand(__written_files, __setup_context):
	catalog = rexlib.ServiceCatalog()
	readers = image.DirectImageReaderProvider(
		image.get_image_read_format_manager(catalog)
	)
	executor = rexlib.concurrency.ThreadPoolExecutor(2)
	source = image.ImageBatchSource(image.ImageSource(readers, executor))
	destination = __setup_destination(__written_files, __setup_context)
	source.read(destination, __written_files).get()
	assert destination.shape[0] == len(__written_files)

def __setup_destination(locations, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		(len(locations), *image.query_core_extents(locations[0].path)),
		rexlib.NumericalType.float32
	)
	return rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, context
	)

@pytest.fixture
def __written_files(tmp_path, __setup_context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		(4, 6), rexlib.NumericalType.float32
	)
	source = rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, __setup_context
	)
	locations = []
	for index in range(3):
		path = str(tmp_path / f'image{index}.mrc')
		image.write(source, path)
		locations.append(image.ImageLocation(path))
	return locations

@pytest.fixture
def __setup_context():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	device_context = rexlib.hardware.DeviceContext(session)
	program_manager = rexlib.dispatch.get_program_manager(catalog)
	dispatcher = rexlib.dispatch.make_eager_dispatcher(program_manager)
	return rexlib.dispatch.ExecutionContext(device_context, dispatcher)
