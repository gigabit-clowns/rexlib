# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image

STACK_DESCRIPTOR = image.ImageDescriptor(
	(4, 4, 6), 2, rexlib.NumericalType.float32
)

def test_a_stack_written_a_batch_at_a_time_reads_back(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((2, 4, 6), __setup_context)
	for first in (0, 2):
		locations = [
			image.ImageLocation(path, first),
			image.ImageLocation(path, first + 1),
		]
		image.write_batch_async(saver, batch, locations).get()
	writers.close(path)
	assert image.query_descriptor(path) == STACK_DESCRIPTOR

def test_the_source_survives_the_write(tmp_path, __setup_context):
	# The write takes the array by value, so a binding that moved rather
	# than shared would leave this one empty.
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((2, 4, 6), __setup_context)
	image.write_batch_async(saver, batch, __setup_locations(path, 2)).get()
	assert batch.shape == (2, 4, 6)

def test_a_completion_reports_when_it_is_done(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((2, 4, 6), __setup_context)
	completion = image.write_batch_async(
		saver, batch, __setup_locations(path, 2)
	)
	completion.wait()
	assert completion.is_ready

def test_a_batch_spans_several_files(tmp_path, __setup_context):
	first = str(tmp_path / 'first.mrcs')
	second = str(tmp_path / 'second.mrcs')
	writers = image.writer_provider()
	writers.declare(first, STACK_DESCRIPTOR)
	writers.declare(second, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((2, 4, 6), __setup_context)
	locations = [
		image.ImageLocation(first, 0),
		image.ImageLocation(second, 3),
	]
	image.write_batch_async(saver, batch, locations).get()
	writers.flush()
	assert image.query_descriptor(first) == STACK_DESCRIPTOR
	assert image.query_descriptor(second) == STACK_DESCRIPTOR

def test_an_empty_batch_is_already_done(tmp_path, __setup_context):
	saver = image.saver(image.writer_provider())
	batch = __setup_array((0, 4, 6), __setup_context)
	assert image.write_batch_async(saver, batch, []).is_ready

def test_a_source_of_the_wrong_batch_size_is_refused(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((3, 4, 6), __setup_context)
	locations = __setup_locations(path, 2)
	with pytest.raises(ValueError):
		image.write_batch_async(saver, batch, locations)

def test_a_path_that_was_not_declared_is_reported(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	saver = image.saver(image.writer_provider())
	batch = __setup_array((2, 4, 6), __setup_context)
	completion = image.write_batch_async(
		saver, batch, __setup_locations(path, 2)
	)
	with pytest.raises(IndexError):
		completion.get()

def test_an_index_past_the_end_of_a_stack_is_reported(
	tmp_path, __setup_context
):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(writers)
	batch = __setup_array((2, 4, 6), __setup_context)
	locations = [
		image.ImageLocation(path, 0),
		image.ImageLocation(path, 4),
	]
	completion = image.write_batch_async(saver, batch, locations)
	with pytest.raises(IndexError):
		completion.get()

def test_runs_on_a_synchronous_executor_too(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	writers = image.writer_provider()
	writers.declare(path, STACK_DESCRIPTOR)
	saver = image.saver(
		writers, executor=rexlib.concurrency.SynchronousExecutor()
	)
	batch = __setup_array((2, 4, 6), __setup_context)
	image.write_batch_async(saver, batch, __setup_locations(path, 2)).get()
	writers.close(path)
	assert image.query_descriptor(path) == STACK_DESCRIPTOR

def test_saver_returns_an_image_saver():
	saver = image.saver(image.writer_provider(), workers=2)
	assert isinstance(saver, image.ExecutorImageSaver)
	assert isinstance(saver, image.ImageSaver)

def test_assembled_by_hand(tmp_path, __setup_context):
	path = str(tmp_path / 'stack.mrcs')
	catalog = rexlib.ServiceCatalog()
	writers = image.ManagedImageWriterProvider(
		image.get_image_write_format_manager(catalog)
	)
	writers.declare(path, STACK_DESCRIPTOR)
	executor = rexlib.concurrency.ThreadPoolExecutor(2)
	saver = image.ExecutorImageSaver(writers, executor)
	batch = __setup_array((2, 4, 6), __setup_context)
	image.write_batch_async(saver, batch, __setup_locations(path, 2)).get()
	writers.close(path)
	assert image.query_descriptor(path) == STACK_DESCRIPTOR

def __setup_locations(path, count):
	return [image.ImageLocation(path, index) for index in range(count)]

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
