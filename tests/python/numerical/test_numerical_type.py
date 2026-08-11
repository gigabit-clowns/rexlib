# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

INTEGER_TYPES = [
	'int8', 'int16', 'int32', 'int64',
	'uint8', 'uint16', 'uint32', 'uint64',
]
FLOAT_TYPES = ['float16', 'float32', 'float64']
COMPLEX_TYPES = ['complex_float16', 'complex_float32', 'complex_float64']

@pytest.mark.parametrize('name', INTEGER_TYPES + FLOAT_TYPES + COMPLEX_TYPES + ['boolean'])
def test_full_supports_numerical_type(name, __setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], getattr(xmipp4.numerical.NumericalType, name)
	)
	result = xmipp4.functional.full(
		descriptor,
		xmipp4.hardware.MemoryResourceAffinity.host,
		1,
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_full_supports_char8_with_a_character(__setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.char8
	)
	result = xmipp4.functional.full(
		descriptor,
		xmipp4.hardware.MemoryResourceAffinity.host,
		'a',
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_full_rejects_a_non_character_char8_value(__setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.char8
	)
	with pytest.raises(TypeError, match='single-character'):
		xmipp4.functional.full(
			descriptor,
			xmipp4.hardware.MemoryResourceAffinity.host,
			1,
			__setup_context
		)

def test_full_reports_the_offending_type(__setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with pytest.raises(TypeError, match='float32'):
		xmipp4.functional.full(
			descriptor,
			xmipp4.hardware.MemoryResourceAffinity.host,
			object(),
			__setup_context
		)

@pytest.mark.parametrize('name', FLOAT_TYPES + COMPLEX_TYPES)
def test_fill_supports_floating_point_type(name, __setup_context):
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], getattr(xmipp4.numerical.NumericalType, name)
	)
	target = xmipp4.functional.empty(
		descriptor,
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	xmipp4.functional.fill(target, 1.5, __setup_context)

@pytest.mark.parametrize('name', INTEGER_TYPES + FLOAT_TYPES)
def test_cast_copy_between_types(name, __setup_context):
	source = xmipp4.functional.ones(
		xmipp4.ndarray.make_contiguous_array_descriptor(
			[2, 2], xmipp4.numerical.NumericalType.float32
		),
		xmipp4.hardware.MemoryResourceAffinity.host,
		__setup_context
	)
	result = xmipp4.functional.cast_copy(
		source,
		getattr(xmipp4.numerical.NumericalType, name),
		__setup_context
	)
	assert isinstance(result, xmipp4.ndarray.Array)

def test_numerical_types_are_distinct():
	names = INTEGER_TYPES + FLOAT_TYPES + COMPLEX_TYPES + ['boolean', 'char8']
	types = {getattr(xmipp4.numerical.NumericalType, name) for name in names}
	assert len(types) == len(names)

def test_numerical_type_converts_to_string():
	assert 'float32' in str(xmipp4.numerical.NumericalType.float32)

def test_numerical_type_exposes_its_value():
	assert isinstance(xmipp4.numerical.NumericalType.float32.value, int)

@pytest.fixture
def __setup_context():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	device_context = xmipp4.hardware.DeviceContext(session)
	program_manager = xmipp4.dispatch.get_program_manager(catalog)
	dispatcher = xmipp4.dispatch.make_eager_dispatcher(program_manager)
	return xmipp4.dispatch.ExecutionContext(device_context, dispatcher)
