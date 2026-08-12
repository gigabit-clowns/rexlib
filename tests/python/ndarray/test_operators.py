# SPDX-License-Identifier: GPL-3.0-only

import copy

import pytest

import xmipp4

BINARY_OPERATORS = [
	('add', lambda x, y: x + y),
	('subtract', lambda x, y: x - y),
	('multiply', lambda x, y: x * y),
	('divide', lambda x, y: x / y),
	('modulo', lambda x, y: x % y),
]

IN_PLACE_OPERATORS = [
	('add', lambda x, y: x.__iadd__(y)),
	('subtract', lambda x, y: x.__isub__(y)),
	('multiply', lambda x, y: x.__imul__(y)),
	('divide', lambda x, y: x.__itruediv__(y)),
	('modulo', lambda x, y: x.__imod__(y)),
]

UNARY_OPERATORS = [
	('negate', lambda x: -x),
	('absolute', abs),
	('positive', lambda x: +x),
]

@pytest.mark.parametrize('name, operator', BINARY_OPERATORS)
def test_binary_operator_returns_an_array(name, operator, __setup_arrays):
	x, y = __setup_arrays
	with xmipp4.device('cpu'):
		assert isinstance(operator(x, y), xmipp4.ndarray.Array)

@pytest.mark.parametrize('name, operator', UNARY_OPERATORS)
def test_unary_operator_returns_an_array(name, operator, __setup_arrays):
	x, _ = __setup_arrays
	with xmipp4.device('cpu'):
		assert isinstance(operator(x), xmipp4.ndarray.Array)

@pytest.mark.parametrize('name, operator', IN_PLACE_OPERATORS)
def test_in_place_operator_keeps_the_same_array(name, operator, __setup_arrays):
	x, y = __setup_arrays
	with xmipp4.device('cpu'):
		assert operator(x, y) is x

@pytest.mark.parametrize('name, operator', BINARY_OPERATORS)
def test_binary_operator_rejects_a_non_array(name, operator, __setup_arrays):
	x, _ = __setup_arrays
	with xmipp4.device('cpu'), pytest.raises(TypeError):
		operator(x, 1)

@pytest.mark.parametrize('name, operator', BINARY_OPERATORS)
def test_binary_operator_needs_an_active_context(name, operator, __setup_arrays):
	x, y = __setup_arrays
	with pytest.raises(RuntimeError):
		operator(x, y)

def test_copy_returns_a_new_array(__setup_arrays):
	x, _ = __setup_arrays
	with xmipp4.device('cpu'):
		result = copy.copy(x)
	assert isinstance(result, xmipp4.ndarray.Array)
	assert result is not x

def test_deepcopy_returns_a_new_array(__setup_arrays):
	x, _ = __setup_arrays
	with xmipp4.device('cpu'):
		result = copy.deepcopy(x)
	assert isinstance(result, xmipp4.ndarray.Array)
	assert result is not x

def test_operators_work_on_arrays_built_by_the_binding(__setup_arrays):
	x, y = __setup_arrays
	assert type(x) is xmipp4.ndarray.Array
	with xmipp4.device('cpu'):
		assert isinstance(x + y, xmipp4.ndarray.Array)

@pytest.fixture
def __setup_arrays():
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with xmipp4.device('cpu'):
		x = xmipp4.functional.ones(
			descriptor, xmipp4.hardware.MemoryResourceAffinity.host
		)
		y = xmipp4.functional.ones(
			descriptor, xmipp4.hardware.MemoryResourceAffinity.host
		)
	return x, y
