# SPDX-License-Identifier: GPL-3.0-only

import numpy
import pytest

import rexlib

IDENTITY = numpy.array(
	[[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]], dtype=numpy.float64
)

@pytest.mark.parametrize("n", [7, 8])
def test_insert_slices_places_a_projection_on_the_central_plane(
	n, __setup_context
):
	volume = numpy.random.default_rng(1).normal(size=(n, n, n))
	slices = numpy.fft.rfft2(volume.sum(axis=0))
	numerator, denominator = __setup_accumulators(n)

	numerator, denominator = rexlib.em.insert_slices(
		numerator,
		denominator,
		rexlib.from_dlpack(slices),
		rexlib.from_dlpack(IDENTITY.copy()),
		rexlib.from_dlpack(numpy.ones(n // 2 + 1)),
		rexlib.em.GriddingWindow.nearest(),
		__setup_context
	)

	weights = numpy.asarray(denominator)
	inserted = weights > 0
	assert numpy.all(weights[inserted] == 1)
	assert not numpy.any(inserted[1:])
	assert numpy.allclose(
		numpy.asarray(numerator)[inserted],
		numpy.fft.rfftn(volume)[inserted],
		atol=1e-9
	)

def test_insert_slices_accumulates_in_place(__setup_context):
	n = 8
	rng = numpy.random.default_rng(2)
	slices = numpy.fft.rfft2(rng.normal(size=(4, n, n)))
	transforms = numpy.repeat(IDENTITY[None], 4, axis=0)
	radial = numpy.ones(n // 2 + 1)
	window = rexlib.em.GriddingWindow.linear()

	whole = __setup_accumulators(n)
	rexlib.em.insert_slices(
		*whole,
		rexlib.from_dlpack(slices),
		rexlib.from_dlpack(transforms),
		rexlib.from_dlpack(radial),
		window,
		__setup_context,
		numerator_out=whole[0],
		denominator_out=whole[1]
	)

	split = __setup_accumulators(n)
	for part in (slice(0, 2), slice(2, 4)):
		rexlib.em.insert_slices(
			*split,
			rexlib.from_dlpack(numpy.ascontiguousarray(slices[part])),
			rexlib.from_dlpack(numpy.ascontiguousarray(transforms[part])),
			rexlib.from_dlpack(radial),
			window,
			__setup_context,
			numerator_out=split[0],
			denominator_out=split[1]
		)

	assert numpy.array_equal(numpy.asarray(split[0]), numpy.asarray(whole[0]))
	assert numpy.array_equal(numpy.asarray(split[1]), numpy.asarray(whole[1]))

def test_insert_weighted_slices_weights_the_denominator(__setup_context):
	n = 8
	slices = numpy.fft.rfft2(numpy.random.default_rng(3).normal(size=(n, n)))
	weights = numpy.full(slices.shape, 0.25)
	_, denominator = rexlib.em.insert_weighted_slices(
		*__setup_accumulators(n),
		rexlib.from_dlpack(slices),
		rexlib.from_dlpack(weights),
		rexlib.from_dlpack(IDENTITY.copy()),
		rexlib.from_dlpack(numpy.ones(n // 2 + 1)),
		rexlib.em.GriddingWindow.nearest(),
		__setup_context
	)

	inserted = numpy.asarray(denominator) > 0
	assert numpy.all(numpy.asarray(denominator)[inserted] == 0.25)

def test_insert_ctf_slices_weights_by_the_ctf(__setup_context):
	n = 8
	slices = numpy.ones((n, n // 2 + 1), dtype=numpy.complex128)
	ctf = numpy.array([15000, 15000, 0, 0], dtype=numpy.float64)
	numerator, denominator = rexlib.em.insert_ctf_slices(
		*__setup_accumulators(n),
		rexlib.from_dlpack(slices),
		rexlib.from_dlpack(IDENTITY.copy()),
		rexlib.from_dlpack(ctf),
		rexlib.from_dlpack(numpy.ones(n // 2 + 1)),
		rexlib.em.GriddingWindow.nearest(),
		rexlib.em.CtfOptics(300, 2.7, 0.1),
		1.5,
		__setup_context
	)

	# The zero frequency carries the amplitude contrast as its CTF.
	assert numpy.asarray(denominator)[0, 0, 0] == pytest.approx(0.01)
	assert numpy.asarray(numerator)[0, 0, 0] == pytest.approx(0.1)

def __setup_accumulators(n):
	shape = (n, n, n // 2 + 1)
	return (
		rexlib.from_dlpack(numpy.zeros(shape, dtype=numpy.complex128)),
		rexlib.from_dlpack(numpy.zeros(shape)),
	)

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
