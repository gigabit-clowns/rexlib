# SPDX-License-Identifier: GPL-3.0-only

import numpy
import pytest

import rexlib

IndexTable = rexlib.IndexTable

def test_default_holds_indices_of_no_coordinates():
	table = IndexTable()
	assert table.rank == 0
	assert len(table) == 0

def test_keeps_the_rank_it_was_given():
	assert IndexTable(3).rank == 3

def test_rank_is_accepted_by_name():
	assert IndexTable(rank=2).rank == 2

def test_starts_empty():
	assert len(IndexTable(2)) == 0

def test_added_indices_are_read_back_in_order():
	table = IndexTable(2)
	table.add((10, 20))
	table.add((30, 40))
	assert len(table) == 2
	assert table[0] == (10, 20)
	assert table[1] == (30, 40)

def test_an_index_is_a_tuple():
	table = IndexTable(2)
	table.add([10, 20])
	assert isinstance(table[0], tuple)

@pytest.mark.parametrize(
	"index",
	[
		pytest.param([10, 20], id="List"),
		pytest.param((10, 20), id="Tuple"),
		pytest.param(range(10, 12), id="Range"),
	]
)
def test_accepts_any_sequence_as_an_index(index):
	table = IndexTable(2)
	table.add(index)
	assert table[0] == tuple(index)

def test_an_index_of_another_rank_is_refused():
	table = IndexTable(2)
	with pytest.raises(ValueError):
		table.add((10, 20, 30))

def test_reading_past_the_end_raises_index_error():
	table = IndexTable(2)
	table.add((10, 20))
	with pytest.raises(IndexError):
		table[1]

def test_can_be_iterated():
	table = IndexTable(2)
	table.add((10, 20))
	table.add((30, 40))
	assert list(table) == [(10, 20), (30, 40)]

def test_clear_drops_the_indices_and_keeps_the_rank():
	table = IndexTable(2)
	table.add((10, 20))
	table.clear()
	assert len(table) == 0
	assert table.rank == 2

def test_reserve_holds_no_index():
	table = IndexTable(2)
	table.reserve(16)
	assert len(table) == 0

def test_is_built_from_an_array_of_one_row_per_index():
	table = IndexTable.from_array(numpy.array([[10, 20, 30], [40, 50, 60]]))
	assert table.rank == 3
	assert list(table) == [(10, 20, 30), (40, 50, 60)]

@pytest.mark.parametrize(
	"dtype",
	[
		numpy.int8, numpy.int16, numpy.int32, numpy.int64,
		numpy.uint8, numpy.uint16, numpy.uint32, numpy.uint64,
		numpy.intp, numpy.uintp,
	]
)
def test_is_built_from_any_integer_type(dtype):
	table = IndexTable.from_array(numpy.array([[1, 2], [3, 4]], dtype=dtype))
	assert list(table) == [(1, 2), (3, 4)]

@pytest.mark.parametrize(
	"indices",
	[
		pytest.param(
			numpy.asfortranarray([[1, 2], [3, 4], [5, 6]]), id="Fortran"
		),
		pytest.param(
			numpy.array([[1, 3, 5], [2, 4, 6]]).T, id="Transposed"
		),
		pytest.param(
			numpy.array([[1, 0, 2], [3, 0, 4], [5, 0, 6]])[:, ::2],
			id="Strided"
		),
	]
)
def test_is_built_from_an_array_that_is_not_contiguous(indices):
	assert list(IndexTable.from_array(indices)) == [(1, 2), (3, 4), (5, 6)]

def test_an_array_of_no_rows_keeps_its_rank():
	table = IndexTable.from_array(numpy.empty((0, 3), dtype=numpy.int64))
	assert table.rank == 3
	assert len(table) == 0

@pytest.mark.parametrize(
	"shape",
	[
		pytest.param((4,), id="One"),
		pytest.param((2, 2, 2), id="Three"),
	]
)
def test_an_array_of_another_number_of_dimensions_is_refused(shape):
	with pytest.raises(ValueError):
		IndexTable.from_array(numpy.zeros(shape, dtype=numpy.int64))

@pytest.mark.parametrize("dtype", [numpy.float32, numpy.float64, numpy.bool_])
def test_an_array_that_does_not_hold_integers_is_refused(dtype):
	with pytest.raises(TypeError):
		IndexTable.from_array(numpy.zeros((2, 2), dtype=dtype))

def test_an_array_holding_a_negative_index_is_refused():
	with pytest.raises(ValueError):
		IndexTable.from_array(numpy.array([[1, 2], [3, -4]]))

class _ArrayLike:
	def __array__(self, dtype=None, copy=None):
		return numpy.array([[1, 2], [3, 4]])

@pytest.mark.parametrize(
	"indices",
	[
		pytest.param([[1, 2], [3, 4]], id="List"),
		pytest.param(_ArrayLike(), id="ArrayProtocol"),
		pytest.param(
			memoryview(numpy.array([[1, 2], [3, 4]])), id="Buffer"
		),
	]
)
def test_what_merely_converts_to_an_array_is_refused(indices):
	with pytest.raises(TypeError):
		IndexTable.from_array(indices)

def test_the_constructor_does_not_take_an_array():
	with pytest.raises(TypeError):
		IndexTable(numpy.array([[1, 2], [3, 4]]))

def test_becomes_an_array_of_one_row_per_index():
	table = IndexTable(2)
	table.add((10, 20))
	table.add((30, 40))
	table.add((50, 60))
	indices = numpy.asarray(table)
	assert indices.shape == (3, 2)
	assert indices.dtype == numpy.uintp
	assert indices.tolist() == [[10, 20], [30, 40], [50, 60]]

def test_an_empty_table_becomes_an_array_of_its_rank():
	assert numpy.asarray(IndexTable(3)).shape == (0, 3)

def test_becomes_an_array_of_the_type_asked_for():
	table = IndexTable(2)
	table.add((10, 20))
	indices = numpy.asarray(table, dtype=numpy.int32)
	assert indices.dtype == numpy.int32
	assert indices.tolist() == [[10, 20]]

def test_the_array_it_becomes_is_a_copy():
	table = IndexTable(2)
	table.add((10, 20))
	indices = numpy.asarray(table)
	indices[0, 0] = 99
	assert table[0] == (10, 20)

def test_becoming_an_array_without_a_copy_is_refused():
	table = IndexTable(2)
	table.add((10, 20))
	with pytest.raises(ValueError):
		numpy.asarray(table, copy=False)

def test_survives_the_round_trip_through_an_array():
	indices = numpy.array([[10, 20], [30, 40]], dtype=numpy.uintp)
	table = IndexTable.from_array(indices)
	assert numpy.array_equal(numpy.asarray(table), indices)
