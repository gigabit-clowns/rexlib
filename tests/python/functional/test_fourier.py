# SPDX-License-Identifier: GPL-3.0-only

import numpy
import pytest

import rexlib

def __setup_real(shape, seed=0):
	return numpy.random.default_rng(seed).normal(size=shape)

def __setup_complex(shape, seed=0):
	rng = numpy.random.default_rng(seed)
	return rng.normal(size=shape) + 1j * rng.normal(size=shape)

@pytest.mark.parametrize("norm", ["backward", "ortho", "forward"])
def test_fft_matches_numpy(norm, __setup_context):
	x = __setup_complex((3, 8, 6))

	result = rexlib.fft(
		rexlib.from_dlpack(x), axes=[-1], norm=norm, context=__setup_context
	)

	assert numpy.allclose(
		numpy.asarray(result), numpy.fft.fft(x, axis=-1, norm=norm)
	)

def test_fft_transforms_the_last_axis_by_default(__setup_context):
	x = __setup_complex((4, 6, 5))

	result = rexlib.fft(rexlib.from_dlpack(x), context=__setup_context)

	assert numpy.allclose(numpy.asarray(result), numpy.fft.fft(x))

def test_fft2_and_ifft2_match_numpy(__setup_context):
	x = __setup_complex((2, 8, 8))

	forward = rexlib.fft2(rexlib.from_dlpack(x), context=__setup_context)
	inverse = rexlib.ifft2(forward, context=__setup_context)

	assert numpy.allclose(numpy.asarray(forward), numpy.fft.fft2(x))
	assert numpy.allclose(numpy.asarray(inverse), x)

def test_fft3_and_ifft3_match_numpy(__setup_context):
	x = __setup_complex((4, 6, 8))

	forward = rexlib.fft3(rexlib.from_dlpack(x), context=__setup_context)
	inverse = rexlib.ifft3(forward, context=__setup_context)

	assert numpy.allclose(numpy.asarray(forward), numpy.fft.fftn(x))
	assert numpy.allclose(numpy.asarray(inverse), x)

def test_ifft_matches_numpy(__setup_context):
	x = __setup_complex((5, 7))

	result = rexlib.ifft(
		rexlib.from_dlpack(x), axes=[0], context=__setup_context
	)

	assert numpy.allclose(numpy.asarray(result), numpy.fft.ifft(x, axis=0))

@pytest.mark.parametrize("extent", [8, 9])
def test_rfft_and_irfft_match_numpy(extent, __setup_context):
	x = __setup_real((3, extent))

	spectrum = rexlib.rfft(
		rexlib.from_dlpack(x), axes=[-1], context=__setup_context
	)
	restored = rexlib.irfft(
		spectrum, extent, axes=[-1], context=__setup_context
	)

	assert numpy.allclose(numpy.asarray(spectrum), numpy.fft.rfft(x))
	assert numpy.allclose(numpy.asarray(restored), x)

def test_rfft2_matches_numpy_over_a_batch(__setup_context):
	x = __setup_real((4, 8, 8)).astype(numpy.float32)

	result = rexlib.rfft2(rexlib.from_dlpack(x), context=__setup_context)

	assert numpy.asarray(result).dtype == numpy.complex64
	assert numpy.allclose(
		numpy.asarray(result), numpy.fft.rfft2(x), rtol=1e-4, atol=1e-4
	)

@pytest.mark.parametrize("extent", [8, 7])
def test_rfft3_and_irfft3_match_numpy(extent, __setup_context):
	x = __setup_real((extent, extent, extent))

	spectrum = rexlib.rfft3(rexlib.from_dlpack(x), context=__setup_context)
	restored = rexlib.irfft3(spectrum, extent, context=__setup_context)

	assert numpy.allclose(numpy.asarray(spectrum), numpy.fft.rfftn(x))
	assert numpy.allclose(numpy.asarray(restored), x)

def test_irfft2_matches_numpy(__setup_context):
	x = __setup_real((2, 6, 6))
	spectrum = numpy.fft.rfft2(x)

	result = rexlib.irfft2(
		rexlib.from_dlpack(spectrum), 6, norm="ortho",
		context=__setup_context
	)

	assert numpy.allclose(
		numpy.asarray(result),
		numpy.fft.irfft2(spectrum, s=(6, 6), norm="ortho")
	)

def test_norm_takes_a_fourier_normalization(__setup_context):
	x = __setup_complex((8,))

	result = rexlib.fft(
		rexlib.from_dlpack(x), norm=rexlib.FourierNormalization.forward,
		context=__setup_context
	)

	assert numpy.allclose(
		numpy.asarray(result), numpy.fft.fft(x, norm="forward")
	)

def test_norm_rejects_an_unknown_name(__setup_context):
	with pytest.raises(KeyError):
		rexlib.fft(
			rexlib.from_dlpack(__setup_complex((8,))), norm="both",
			context=__setup_context
		)

@pytest.mark.parametrize("shape", [(6, 7), (5,)])
def test_fftshift_and_ifftshift_match_numpy(shape, __setup_context):
	x = __setup_real(shape)

	shifted = rexlib.fftshift(rexlib.from_dlpack(x), context=__setup_context)
	back = rexlib.ifftshift(shifted, context=__setup_context)
	along = rexlib.fftshift(
		rexlib.from_dlpack(x), axes=[0], context=__setup_context
	)

	assert numpy.array_equal(numpy.asarray(shifted), numpy.fft.fftshift(x))
	assert numpy.array_equal(numpy.asarray(back), x)
	assert numpy.array_equal(
		numpy.asarray(along), numpy.fft.fftshift(x, axes=0)
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
