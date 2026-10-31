# SPDX-License-Identifier: GPL-3.0-only

import math

import numpy
import pytest

import rexlib

def test_standardize_background_2d_matches_a_reference(__setup_context):
	images = __setup_images()

	result = rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images),
		__setup_seeds(3),
		4,
		math.inf,
		__setup_context
	)

	assert isinstance(result, rexlib.Array)
	assert numpy.allclose(
		numpy.asarray(result), __reference(images, 4), rtol=1e-10
	)

def test_standardize_background_takes_axes_from_the_end(__setup_context):
	images = __setup_images()
	seeds = __setup_seeds(3)

	named = rexlib.em.standardize_background(
		rexlib.from_dlpack(images), seeds, [-2, -1], 4, (-3, 3),
		__setup_context
	)
	numbered = rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images), seeds, 4, (-3, 3), __setup_context
	)

	assert numpy.array_equal(numpy.asarray(named), numpy.asarray(numbered))

def test_standardize_background_replaces_what_is_not_finite(__setup_context):
	images = __setup_images()
	images[0, 0, :] = math.nan
	images[1, 5, 5] = math.inf

	result = numpy.asarray(rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images), __setup_seeds(3), 4, 3,
		__setup_context
	))

	assert numpy.all(numpy.isfinite(result))

def test_standardize_background_draws_by_seed(__setup_context):
	images = numpy.full((2, 8, 8), math.nan)

	def draw(seeds):
		return numpy.asarray(rexlib.em.standardize_background_2d(
			rexlib.from_dlpack(images.copy()),
			rexlib.from_dlpack(numpy.asarray(seeds, dtype=numpy.uint64)),
			2, 3, __setup_context
		))

	assert numpy.array_equal(draw([1, 2]), draw([1, 2]))
	assert not numpy.array_equal(draw([1, 2])[0], draw([3, 2])[0])
	assert numpy.array_equal(draw([1, 2])[1], draw([3, 2])[1])

def test_standardize_background_3d_keeps_the_shape(__setup_context):
	volumes = numpy.random.default_rng(7).normal(size=(2, 6, 6, 6))

	result = rexlib.em.standardize_background_3d(
		rexlib.from_dlpack(volumes), __setup_seeds(2), 2, 3,
		__setup_context
	)

	assert numpy.asarray(result).shape == (2, 6, 6, 6)

def test_standardize_background_writes_into_out(__setup_context):
	images = __setup_images()
	out = rexlib.from_dlpack(numpy.zeros_like(images))

	result = rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images), __setup_seeds(3), 4, 3,
		__setup_context, out=out
	)

	assert numpy.array_equal(numpy.asarray(out), numpy.asarray(result))

def test_standardize_background_uses_the_active_context():
	images = __setup_images()
	with rexlib.device('cpu'):
		result = rexlib.em.standardize_background_2d(
			rexlib.from_dlpack(images), __setup_seeds(3), 4, 3
		)
	assert numpy.asarray(result).shape == images.shape

def test_standardize_background_reads_one_bound_as_both(__setup_context):
	images = __setup_images()
	images[0, 0, 0] = 1000
	seeds = __setup_seeds(3)

	symmetric = rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images), seeds, 4, 2.5, __setup_context
	)
	paired = rexlib.em.standardize_background_2d(
		rexlib.from_dlpack(images), seeds, 4, (-2.5, 2.5), __setup_context
	)

	assert numpy.array_equal(numpy.asarray(symmetric), numpy.asarray(paired))

@pytest.mark.parametrize(
	"bounds",
	[(0.0, 3.0), (-3.0, 0.0), (3.0, -3.0), (math.nan, 3.0), 0.0, -3.0],
)
def test_standardize_background_refuses_bounds_around_no_mean(
	__setup_context, bounds
):
	images = rexlib.from_dlpack(__setup_images())
	seeds = __setup_seeds(3)
	with pytest.raises(ValueError):
		rexlib.em.standardize_background_2d(
			images, seeds, 4, bounds, __setup_context
		)

def test_standardize_background_refuses_bounds_of_another_length(
	__setup_context
):
	images = rexlib.from_dlpack(__setup_images())
	seeds = __setup_seeds(3)
	with pytest.raises(ValueError):
		rexlib.em.standardize_background_2d(
			images, seeds, 4, (-3.0, 0.0, 3.0), __setup_context
		)

def __reference(images, radius):
	height, width = images.shape[-2:]
	rows, columns = numpy.indices((height, width))
	background = (
		(rows - height // 2) ** 2 + (columns - width // 2) ** 2 > radius ** 2
	)

	result = numpy.empty_like(images)
	for index, image in enumerate(images):
		values = image[background]
		result[index] = (image - values.mean()) / values.std()
	return result

def __setup_images():
	return numpy.random.default_rng(3).normal(100, 5, size=(3, 16, 16))

def __setup_seeds(count):
	return rexlib.from_dlpack(numpy.arange(count, dtype=numpy.uint64))

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
