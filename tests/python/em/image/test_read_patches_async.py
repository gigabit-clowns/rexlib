# SPDX-License-Identifier: GPL-3.0-only

import numpy
import pytest

import rexlib

image = rexlib.em.image

def test_crops_patches_given_as_a_list_of_centres(
	__written_image, __setup_context
):
	loader = image.loader()
	destination = __setup_array((2, 8, 8), __setup_context)
	image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image),
		[(10, 10), (20, 30)]
	).get()
	assert destination.shape == (2, 8, 8)

def test_crops_patches_given_as_an_index_table(
	__written_image, __setup_context
):
	loader = image.loader()
	centres = rexlib.IndexTable(2)
	centres.add((10, 10))
	centres.add((20, 30))
	destination = __setup_array((2, 8, 8), __setup_context)
	image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image), centres
	).get()
	assert destination.shape == (2, 8, 8)

def test_crops_patches_given_as_an_array_of_centres(
	__written_image, __setup_context
):
	loader = image.loader()
	centres = numpy.array([[10, 10], [20, 30]])
	destination = __setup_array((2, 8, 8), __setup_context)
	image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image), centres
	).get()
	assert destination.shape == (2, 8, 8)

def test_the_destination_survives_the_read(__written_image, __setup_context):
	loader = image.loader()
	destination = __setup_array((1, 8, 8), __setup_context)
	image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image), [(10, 10)]
	).get()
	assert destination.shape == (1, 8, 8)

@pytest.mark.parametrize(
	"centre",
	[
		pytest.param((0, 0), id="Before the origin"),
		pytest.param((31, 47), id="Past the far corner"),
		pytest.param((100, 100), id="Outside the image"),
	]
)
def test_a_patch_over_a_border_is_clipped_rather_than_refused(
	centre, __written_image, __setup_context
):
	loader = image.loader()
	destination = __setup_array((1, 8, 8), __setup_context)
	completion = image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image), [centre]
	)
	completion.get()
	assert completion.is_ready

def test_crops_out_of_one_image_of_a_stack(__written_stack, __setup_context):
	loader = image.loader()
	destination = __setup_array((1, 4, 4), __setup_context)
	image.read_patches_async(
		loader, destination, image.ImageLocation(__written_stack, 1),
		[(2, 3)]
	).get()
	assert destination.shape == (1, 4, 4)

def test_an_empty_batch_is_already_done(__written_image, __setup_context):
	loader = image.loader()
	destination = __setup_array((0, 8, 8), __setup_context)
	completion = image.read_patches_async(
		loader, destination, image.ImageLocation(__written_image), []
	)
	assert completion.is_ready

def test_a_destination_of_the_wrong_batch_size_is_refused(
	__written_image, __setup_context
):
	loader = image.loader()
	destination = __setup_array((3, 8, 8), __setup_context)
	location = image.ImageLocation(__written_image)
	with pytest.raises(ValueError):
		image.read_patches_async(
			loader, destination, location, [(10, 10), (20, 30)]
		)

def test_a_centre_of_another_rank_is_refused(__written_image, __setup_context):
	loader = image.loader()
	destination = __setup_array((1, 8, 8), __setup_context)
	location = image.ImageLocation(__written_image)
	with pytest.raises(ValueError):
		image.read_patches_async(
			loader, destination, location, [(10, 10, 10)]
		)

def test_an_index_table_of_another_rank_is_refused(
	__written_image, __setup_context
):
	loader = image.loader()
	centres = rexlib.IndexTable(3)
	centres.add((10, 10, 10))
	destination = __setup_array((1, 8, 8), __setup_context)
	location = image.ImageLocation(__written_image)
	with pytest.raises(ValueError):
		image.read_patches_async(loader, destination, location, centres)

def test_an_array_of_centres_of_another_rank_is_refused(
	__written_image, __setup_context
):
	loader = image.loader()
	destination = __setup_array((1, 8, 8), __setup_context)
	location = image.ImageLocation(__written_image)
	centres = numpy.array([[10, 10, 10]])
	with pytest.raises(ValueError):
		image.read_patches_async(loader, destination, location, centres)

def test_centres_that_are_neither_table_array_nor_sequence_are_refused(
	__written_image, __setup_context
):
	loader = image.loader()
	destination = __setup_array((1, 8, 8), __setup_context)
	location = image.ImageLocation(__written_image)
	with pytest.raises(TypeError):
		image.read_patches_async(loader, destination, location, 10)

def __setup_array(shape, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.zeros(
		descriptor, rexlib.hardware.MemoryResourceAffinity.host, context
	)

@pytest.fixture
def __written_image(tmp_path, __setup_context):
	path = str(tmp_path / 'image.mrc')
	image.write_single(__setup_array((32, 48), __setup_context), path)
	return path

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
