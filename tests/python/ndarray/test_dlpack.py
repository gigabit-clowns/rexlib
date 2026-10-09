# SPDX-License-Identifier: GPL-3.0-only

import ctypes
import gc
import weakref

import numpy
import pytest

import rexlib

HOST = rexlib.hardware.MemoryResourceAffinity.host
CPU = (1, 0)
CUDA = (2, 0)
FLOAT32 = (2, 32, 1)
BRAIN_FLOAT16 = (4, 16, 1)
FLOAT32_PAIR = (2, 32, 2)
READ_ONLY = 1

numpy_speaks_dlpack_1 = pytest.mark.skipif(
	numpy.lib.NumpyVersion(numpy.__version__) < '2.1.0',
	reason='numpy takes and gives versioned tensors from 2.1 on'
)

DATA_TYPES = [
	(rexlib.NumericalType.boolean, numpy.bool_),
	(rexlib.NumericalType.int8, numpy.int8),
	(rexlib.NumericalType.uint8, numpy.uint8),
	(rexlib.NumericalType.int16, numpy.int16),
	(rexlib.NumericalType.uint16, numpy.uint16),
	(rexlib.NumericalType.int32, numpy.int32),
	(rexlib.NumericalType.uint32, numpy.uint32),
	(rexlib.NumericalType.int64, numpy.int64),
	(rexlib.NumericalType.uint64, numpy.uint64),
	(rexlib.NumericalType.float16, numpy.float16),
	(rexlib.NumericalType.float32, numpy.float32),
	(rexlib.NumericalType.float64, numpy.float64),
	(rexlib.NumericalType.complex_float32, numpy.complex64),
	(rexlib.NumericalType.complex_float64, numpy.complex128),
]

class Exported:
	"""Hands a capsule that already exists to a consumer of DLPack."""

	def __init__(self, capsule):
		self.capsule = capsule

	def __dlpack__(self, **_):
		return self.capsule

	def __dlpack_device__(self):
		return CPU

class OldExporter:
	"""Exports like a library from before DLPack 1.0: it takes no argument."""

	def __init__(self, source):
		self.source = source

	def __dlpack__(self):
		return self.source.__dlpack__()

class DLDevice(ctypes.Structure):
	_fields_ = (
		('device_type', ctypes.c_int),
		('device_id', ctypes.c_int32),
	)

class DLDataType(ctypes.Structure):
	_fields_ = (
		('code', ctypes.c_uint8),
		('bits', ctypes.c_uint8),
		('lanes', ctypes.c_uint16),
	)

class DLTensor(ctypes.Structure):
	_fields_ = (
		('data', ctypes.c_void_p),
		('device', DLDevice),
		('ndim', ctypes.c_int32),
		('dtype', DLDataType),
		('shape', ctypes.POINTER(ctypes.c_int64)),
		('strides', ctypes.POINTER(ctypes.c_int64)),
		('byte_offset', ctypes.c_uint64),
	)

Deleter = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

class DLManagedTensor(ctypes.Structure):
	_fields_ = (
		('dl_tensor', DLTensor),
		('manager_ctx', ctypes.c_void_p),
		('deleter', Deleter),
	)

class DLPackVersion(ctypes.Structure):
	_fields_ = (
		('major', ctypes.c_uint32),
		('minor', ctypes.c_uint32),
	)

class DLManagedTensorVersioned(ctypes.Structure):
	_fields_ = (
		('version', DLPackVersion),
		('manager_ctx', ctypes.c_void_p),
		('deleter', Deleter),
		('flags', ctypes.c_uint64),
		('dl_tensor', DLTensor),
	)

UNVERSIONED_NAME = b'dltensor'
VERSIONED_NAME = b'dltensor_versioned'

new_capsule = ctypes.PYFUNCTYPE(
	ctypes.py_object, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p
)(('PyCapsule_New', ctypes.pythonapi))

class HandMade:
	"""Exports a tensor of floats built by hand.

	It counts its releases and keeps what it was asked for. It also gives
	what a library does not export, or not in every version: a tensor
	without strides, with a byte offset, without a deleter, on another
	device, of a data type no array has, read only or of another version.
	"""

	def __init__(
		self, values, shape, *,
		strides=None, byte_offset=0, device=CPU, data_type=FLOAT32,
		has_deleter=True, version=None, flags=0
	):
		self.released = 0
		self.requested = None
		self.capsule = None
		self.__values = (ctypes.c_float * len(values))(*values)
		self.__shape = (ctypes.c_int64 * len(shape))(*shape)
		self.__strides = (
			None if strides is None
			else (ctypes.c_int64 * len(strides))(*strides)
		)
		self.__deleter = Deleter(self.__release) if has_deleter else Deleter()

		tensor = DLTensor(
			ctypes.addressof(self.__values),
			DLDevice(*device),
			len(shape),
			DLDataType(*data_type),
			self.__shape,
			self.__strides,
			byte_offset,
		)
		if version is None:
			self.__name = UNVERSIONED_NAME
			self.__managed = DLManagedTensor(tensor, None, self.__deleter)
		else:
			self.__name = VERSIONED_NAME
			self.__managed = DLManagedTensorVersioned(
				DLPackVersion(*version), None, self.__deleter, flags, tensor
			)

	def __release(self, _):
		self.released += 1

	def __dlpack__(self, **requested):
		self.requested = requested
		self.capsule = new_capsule(
			ctypes.addressof(self.__managed), self.__name, None
		)
		return self.capsule

def test_reports_the_host_as_its_device(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	assert array.__dlpack_device__() == CPU

@pytest.mark.parametrize(('data_type', 'dtype'), DATA_TYPES)
def test_numpy_sees_the_data_type(data_type, dtype, __setup_context):
	array = __setup_empty([2, 3], data_type, __setup_context)
	assert numpy.from_dlpack(array).dtype == dtype

def test_numpy_sees_the_shape_and_the_values(__setup_context):
	array = __setup_full([2, 3], 2.5, __setup_context)
	expected = numpy.full((2, 3), 2.5, dtype=numpy.float32)
	assert numpy.array_equal(numpy.from_dlpack(array), expected)

def test_the_memory_is_shared_and_not_copied(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	assert numpy.shares_memory(
		numpy.from_dlpack(array), numpy.asarray(array)
	)

def test_a_write_by_an_operation_is_seen_by_the_consumer(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	view = numpy.from_dlpack(array)
	rexlib.fill(array, 5, __setup_context)
	assert numpy.array_equal(view, numpy.full((2, 3), 5, dtype=numpy.float32))

def test_the_consumer_keeps_the_memory_alive(__setup_context):
	view = numpy.from_dlpack(__setup_ones([2, 3], __setup_context))
	gc.collect()
	assert view.sum() == 6

def test_a_capsule_nobody_takes_is_released(__setup_context):
	capsule = __setup_ones([2, 3], __setup_context).__dlpack__()
	del capsule
	gc.collect()

def test_the_capsule_is_unversioned_by_default(__setup_context):
	capsule = __setup_ones([2, 3], __setup_context).__dlpack__()
	assert '"dltensor"' in repr(capsule)

def test_the_capsule_is_unversioned_for_an_old_consumer(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	capsule = array.__dlpack__(max_version=(0, 8))
	assert '"dltensor"' in repr(capsule)

def test_the_capsule_is_versioned_for_a_consumer_that_can_take_it(
	__setup_context
):
	array = __setup_ones([2, 3], __setup_context)
	capsule = array.__dlpack__(max_version=(1, 0))
	assert '"dltensor_versioned"' in repr(capsule)

@numpy_speaks_dlpack_1
def test_numpy_takes_a_versioned_capsule(__setup_context):
	array = __setup_full([2, 3], 2.5, __setup_context)
	exported = Exported(array.__dlpack__(max_version=(1, 0)))
	expected = numpy.full((2, 3), 2.5, dtype=numpy.float32)
	assert numpy.array_equal(numpy.from_dlpack(exported), expected)

def test_the_device_the_array_is_on_can_be_asked_for(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	exported = Exported(array.__dlpack__(dl_device=CPU))
	assert numpy.from_dlpack(exported).shape == (2, 3)

def test_another_device_is_refused(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	with pytest.raises(BufferError):
		array.__dlpack__(dl_device=CUDA)

def test_a_stream_is_refused(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	with pytest.raises(ValueError, match='stream'):
		array.__dlpack__(stream=1)

def test_a_copy_is_made_when_asked_for(__setup_context):
	array = __setup_ones([2, 3], __setup_context)
	with rexlib.device('cpu'):
		exported = Exported(array.__dlpack__(copy=True))
	copied = numpy.from_dlpack(exported)
	assert numpy.array_equal(copied, numpy.ones((2, 3), dtype=numpy.float32))
	assert not numpy.shares_memory(copied, numpy.asarray(array))

def test_a_type_dlpack_cannot_name_is_refused(__setup_context):
	array = __setup_empty(
		[2, 3], rexlib.NumericalType.char8, __setup_context
	)
	with pytest.raises(BufferError):
		array.__dlpack__()

def test_torch_shares_the_memory(__setup_context):
	torch = pytest.importorskip('torch')
	array = __setup_full([2, 3], 2.5, __setup_context)
	tensor = torch.from_dlpack(array)
	assert tensor.shape == (2, 3)
	assert tensor.dtype == torch.float32
	assert bool((tensor == 2.5).all())
	tensor[0, 0] = 7
	assert numpy.asarray(array)[0, 0] == 7

def test_jax_reads_the_values(__setup_context):
	jax_numpy = pytest.importorskip('jax.numpy')
	array = __setup_full([2, 3], 2.5, __setup_context)
	values = jax_numpy.from_dlpack(array)
	assert values.shape == (2, 3)
	assert bool((values == 2.5).all())

@pytest.mark.parametrize(('data_type', 'dtype'), DATA_TYPES)
def test_an_array_takes_the_data_type_of_numpy(data_type, dtype):
	array = rexlib.from_dlpack(numpy.zeros((2, 3), dtype=dtype))
	assert array.data_type == data_type

def test_an_array_takes_the_shape_and_the_values_of_numpy():
	source = numpy.arange(6, dtype=numpy.float32).reshape(2, 3)
	array = rexlib.from_dlpack(source)
	assert array.shape == (2, 3)
	assert numpy.array_equal(numpy.asarray(array), source)

def test_an_array_shares_the_memory_it_takes():
	source = numpy.zeros((2, 3), dtype=numpy.float32)
	array = rexlib.from_dlpack(source)
	assert numpy.shares_memory(numpy.asarray(array), source)

def test_a_write_by_an_operation_is_seen_by_the_source(__setup_context):
	source = numpy.zeros((2, 3), dtype=numpy.float32)
	rexlib.fill(rexlib.from_dlpack(source), 5, __setup_context)
	assert numpy.array_equal(
		source, numpy.full((2, 3), 5, dtype=numpy.float32)
	)

def test_a_write_by_the_source_is_seen_by_an_operation(__setup_context):
	source = numpy.zeros((2, 3), dtype=numpy.float32)
	array = rexlib.from_dlpack(source)
	source[...] = 3
	result = rexlib.add(array, array, __setup_context)
	assert numpy.array_equal(
		numpy.asarray(result), numpy.full((2, 3), 6, dtype=numpy.float32)
	)

def test_an_array_keeps_alive_the_memory_it_takes():
	source = numpy.arange(6, dtype=numpy.float32)
	alive = weakref.ref(source)
	array = rexlib.from_dlpack(source)
	del source
	gc.collect()
	assert alive() is not None
	assert numpy.asarray(array).sum() == 15
	del array
	gc.collect()
	assert alive() is None

def test_the_source_outlives_the_array():
	source = numpy.arange(6, dtype=numpy.float32)
	array = rexlib.from_dlpack(source)
	del array
	gc.collect()
	assert source.sum() == 15

@pytest.mark.parametrize(
	'view',
	[
		lambda source: source[::2, 1::2],
		lambda source: source.T,
		lambda source: source[::-1],
		lambda source: source[1:, ::-2],
		lambda source: source[2, 3, ...],
		lambda source: source[:0],
	],
	ids=['stepped', 'transposed', 'reversed', 'offset', 'scalar', 'empty']
)
def test_a_view_of_numpy_survives_the_round_trip(view):
	source = view(numpy.arange(24, dtype=numpy.float32).reshape(4, 6))
	back = numpy.asarray(rexlib.from_dlpack(source))
	assert back.shape == source.shape
	assert back.strides == source.strides
	assert numpy.array_equal(back, source)
	assert numpy.shares_memory(back, source) == (source.size != 0)

def test_an_operation_reads_a_reversed_view(__setup_context):
	source = numpy.arange(6, dtype=numpy.float32)[::-1]
	array = rexlib.from_dlpack(source)
	result = rexlib.add(array, array, __setup_context)
	assert numpy.array_equal(numpy.asarray(result), 2 * source)

def test_the_storage_is_as_large_as_what_the_tensor_reaches(__setup_context):
	source = numpy.zeros(6, dtype=numpy.float32)[::2]
	fitting = rexlib.empty(
		__setup_descriptor([5]), HOST, __setup_context,
		out=rexlib.from_dlpack(source)
	)
	larger = rexlib.empty(
		__setup_descriptor([6]), HOST, __setup_context,
		out=rexlib.from_dlpack(source)
	)
	assert numpy.shares_memory(numpy.asarray(fitting), source)
	assert not numpy.shares_memory(numpy.asarray(larger), source)

def test_numpy_takes_back_what_it_gave():
	source = numpy.arange(6, dtype=numpy.float32)
	back = numpy.from_dlpack(rexlib.from_dlpack(source))
	assert numpy.array_equal(back, source)
	assert numpy.shares_memory(back, source)

def test_an_array_takes_the_memory_of_another(__setup_context):
	source = __setup_full([2, 3], 2.5, __setup_context)
	array = rexlib.from_dlpack(source)
	assert numpy.shares_memory(numpy.asarray(array), numpy.asarray(source))

def test_a_writable_source_is_shared_when_no_copy_is_allowed():
	source = numpy.zeros((2, 3), dtype=numpy.float32)
	array = rexlib.from_dlpack(source, copy=False)
	assert numpy.shares_memory(numpy.asarray(array), source)

@pytest.mark.parametrize('copy', [None, False, True])
def test_the_source_is_asked_for_what_the_caller_wants(copy):
	source = HandMade([1, 2, 3], [3])
	array = rexlib.from_dlpack(source, copy=copy)
	assert source.requested['copy'] is copy
	assert source.requested['max_version'][0] == 1
	del array

@numpy_speaks_dlpack_1
def test_numpy_copies_when_asked_to():
	source = numpy.arange(6, dtype=numpy.float32)
	copied = numpy.asarray(rexlib.from_dlpack(source, copy=True))
	assert numpy.array_equal(copied, source)
	assert not numpy.shares_memory(copied, source)

def test_a_versioned_tensor_is_taken():
	source = HandMade([1, 2, 3], [3], version=(1, 0))
	array = rexlib.from_dlpack(source)
	assert numpy.asarray(array).tolist() == [1, 2, 3]
	assert '"used_dltensor_versioned"' in repr(source.capsule)
	del array
	gc.collect()
	assert source.released == 1

def test_a_tensor_of_another_major_version_is_refused():
	source = HandMade([1, 2, 3], [3], version=(2, 0))
	with pytest.raises(BufferError, match='version 2'):
		rexlib.from_dlpack(source)
	assert source.released == 0

def test_a_read_only_tensor_is_copied_and_given_back():
	source = HandMade([1, 2, 3], [3], version=(1, 0), flags=READ_ONLY)
	with rexlib.device('cpu'):
		copied = rexlib.from_dlpack(source)
	gc.collect()
	assert numpy.asarray(copied).tolist() == [1, 2, 3]
	assert source.released == 1

def test_a_read_only_tensor_is_refused_when_no_copy_is_allowed():
	source = HandMade([1, 2, 3], [3], version=(1, 0), flags=READ_ONLY)
	with pytest.raises(BufferError, match='read only'):
		rexlib.from_dlpack(source, copy=False)
	assert source.released == 0

@numpy_speaks_dlpack_1
def test_a_read_only_array_of_numpy_is_copied():
	source = numpy.arange(6, dtype=numpy.float32)
	source.flags.writeable = False
	with rexlib.device('cpu'):
		copied = numpy.asarray(rexlib.from_dlpack(source))
	assert numpy.array_equal(copied, source)
	assert not numpy.shares_memory(copied, source)

def test_a_source_that_is_not_aligned_is_refused():
	source = numpy.frombuffer(
		bytearray(13), dtype=numpy.float32, count=3, offset=1
	)
	with pytest.raises(BufferError, match='aligned'):
		rexlib.from_dlpack(source)

def test_an_old_exporter_is_taken():
	source = numpy.arange(6, dtype=numpy.float32)
	array = rexlib.from_dlpack(OldExporter(source))
	assert numpy.shares_memory(numpy.asarray(array), source)

def test_an_old_exporter_cannot_be_asked_for_a_copy():
	exporter = OldExporter(numpy.arange(6, dtype=numpy.float32))
	with pytest.raises(TypeError):
		rexlib.from_dlpack(exporter, copy=True)

def test_an_object_that_does_not_export_is_refused():
	with pytest.raises(AttributeError):
		rexlib.from_dlpack([1, 2, 3])

def test_a_tensor_is_released_when_its_array_dies():
	source = HandMade([1, 2, 3], [3])
	array = rexlib.from_dlpack(source)
	assert source.released == 0
	del array
	gc.collect()
	assert source.released == 1

def test_a_tensor_is_released_once_when_its_last_array_dies(__setup_context):
	source = HandMade([1, 2, 3], [3])
	first = rexlib.from_dlpack(source)
	second = rexlib.empty(
		__setup_descriptor([3]), HOST, __setup_context, out=first
	)
	del first
	gc.collect()
	assert source.released == 0
	del second
	gc.collect()
	assert source.released == 1

def test_a_view_of_the_array_keeps_the_tensor():
	source = HandMade([1, 2, 3], [3])
	view = numpy.asarray(rexlib.from_dlpack(source))
	gc.collect()
	assert source.released == 0
	assert view.tolist() == [1, 2, 3]
	del view
	gc.collect()
	assert source.released == 1

def test_a_tensor_without_a_deleter_is_taken():
	source = HandMade([1, 2, 3], [3], has_deleter=False)
	array = rexlib.from_dlpack(source)
	assert numpy.asarray(array).tolist() == [1, 2, 3]
	del array
	gc.collect()

def test_a_capsule_is_marked_once_its_tensor_is_taken():
	source = HandMade([1, 2, 3], [3])
	array = rexlib.from_dlpack(source)
	assert '"used_dltensor"' in repr(source.capsule)
	del array

def test_a_tensor_is_taken_out_of_its_capsule_only_once():
	source = HandMade([1, 2, 3], [3])
	array = rexlib.from_dlpack(source)
	emptied = Exported(source.capsule)
	with pytest.raises(ValueError, match='only once'):
		rexlib.from_dlpack(emptied)
	del array
	gc.collect()
	assert source.released == 1

def test_a_tensor_without_strides_is_contiguous():
	source = HandMade([1, 2, 3, 4, 5, 6], [2, 3])
	array = numpy.asarray(rexlib.from_dlpack(source))
	assert array.tolist() == [[1, 2, 3], [4, 5, 6]]

def test_the_strides_of_a_tensor_count_elements():
	source = HandMade([1, 2, 3, 4, 5, 6], [2, 2], strides=[3, 2])
	array = numpy.asarray(rexlib.from_dlpack(source))
	assert array.tolist() == [[1, 3], [4, 6]]

def test_the_byte_offset_of_a_tensor_is_skipped():
	source = HandMade([1, 2, 3], [2], byte_offset=4)
	array = numpy.asarray(rexlib.from_dlpack(source))
	assert array.tolist() == [2, 3]

def test_a_tensor_on_another_device_is_refused():
	source = HandMade([1, 2, 3], [3], device=CUDA)
	with pytest.raises(BufferError, match='host memory'):
		rexlib.from_dlpack(source)

@pytest.mark.parametrize('data_type', [BRAIN_FLOAT16, FLOAT32_PAIR])
def test_a_tensor_of_a_data_type_no_array_has_is_refused(data_type):
	source = HandMade([1, 2, 3], [3], data_type=data_type)
	with pytest.raises(BufferError, match='data type'):
		rexlib.from_dlpack(source)

def test_a_refused_tensor_stays_with_its_capsule():
	source = HandMade([1, 2, 3], [3], device=CUDA)
	with pytest.raises(BufferError):
		rexlib.from_dlpack(source)
	assert '"dltensor"' in repr(source.capsule)
	assert source.released == 0

def test_an_array_shares_the_memory_of_torch(__setup_context):
	torch = pytest.importorskip('torch')
	tensor = torch.full((2, 3), 2.5)
	array = rexlib.from_dlpack(tensor)
	assert array.shape == (2, 3)
	assert array.data_type == rexlib.NumericalType.float32
	rexlib.fill(array, 7, __setup_context)
	assert bool((tensor == 7).all())

def test_an_array_outlives_the_tensor_of_torch():
	torch = pytest.importorskip('torch')
	array = rexlib.from_dlpack(torch.full((2, 3), 2.5))
	gc.collect()
	assert numpy.asarray(array).sum() == 15

def test_an_array_reads_the_values_of_jax():
	jax_numpy = pytest.importorskip('jax.numpy')
	array = rexlib.from_dlpack(jax_numpy.full((2, 3), 2.5))
	assert numpy.array_equal(
		numpy.asarray(array), numpy.full((2, 3), 2.5, dtype=numpy.float32)
	)

def __setup_descriptor(shape):
	return rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)

def __setup_empty(shape, data_type, context):
	descriptor = rexlib.make_contiguous_array_descriptor(shape, data_type)
	return rexlib.empty(descriptor, HOST, context)

def __setup_ones(shape, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.ones(descriptor, HOST, context)

def __setup_full(shape, value, context):
	descriptor = rexlib.make_contiguous_array_descriptor(
		shape, rexlib.NumericalType.float32
	)
	return rexlib.full(descriptor, HOST, value, context)

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
