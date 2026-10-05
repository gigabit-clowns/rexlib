# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

image = rexlib.em.image
ImageDescriptor = image.ImageDescriptor

def test_holds_what_it_was_constructed_with():
	descriptor = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert descriptor.extents == (6, 3, 4)
	assert descriptor.core_rank == 2
	assert descriptor.data_type == rexlib.NumericalType.float32

def test_extents_are_a_tuple():
	descriptor = ImageDescriptor([3, 4], 2, rexlib.NumericalType.float32)
	assert isinstance(descriptor.extents, tuple)

@pytest.mark.parametrize(
	"extents",
	[
		pytest.param([6, 3, 4], id="List"),
		pytest.param((6, 3, 4), id="Tuple"),
		pytest.param(range(2, 5), id="Range"),
	]
)
def test_accepts_any_sequence_of_extents(extents):
	descriptor = ImageDescriptor(extents, 2, rexlib.NumericalType.float32)
	assert descriptor.extents == tuple(extents)

def test_arguments_are_accepted_by_name():
	descriptor = ImageDescriptor(
		extents=(6, 3, 4), core_rank=2, data_type=rexlib.NumericalType.int16
	)
	assert descriptor.data_type == rexlib.NumericalType.int16

@pytest.mark.parametrize(
	"core_rank",
	[
		pytest.param(0, id="No core axis"),
		pytest.param(4, id="More core axes than extents"),
	]
)
def test_refuses_a_core_rank_its_extents_cannot_hold(core_rank):
	with pytest.raises(ValueError):
		ImageDescriptor((6, 3, 4), core_rank, rexlib.NumericalType.float32)

def test_core_extents_leave_out_the_axes_a_file_stacks_along():
	stack = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	volume = ImageDescriptor((6, 3, 4), 3, rexlib.NumericalType.float32)
	assert image.get_core_extents(stack) == (3, 4)
	assert image.get_core_extents(volume) == (6, 3, 4)

def test_core_extents_are_a_tuple():
	descriptor = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert isinstance(image.get_core_extents(descriptor), tuple)

def test_equal_descriptors_compare_equal():
	first = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	second = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert first == second

@pytest.mark.parametrize(
	"other",
	[
		pytest.param(
			ImageDescriptor((5, 3, 4), 2, rexlib.NumericalType.float32),
			id="Different extents"
		),
		pytest.param(
			ImageDescriptor((6, 3, 4), 3, rexlib.NumericalType.float32),
			id="Different core rank"
		),
		pytest.param(
			ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.int16),
			id="Different data type"
		),
	]
)
def test_different_descriptors_compare_not_equal(other):
	descriptor = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert descriptor != other

def test_equal_descriptors_hash_equal():
	first = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	second = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert hash(first) == hash(second)

def test_usable_as_a_dictionary_key():
	key = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	table = {key: 'value'}
	same = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert table[same] == 'value'

def test_repr_names_the_type():
	descriptor = ImageDescriptor((6, 3, 4), 2, rexlib.NumericalType.float32)
	assert repr(descriptor).startswith('ImageDescriptor(')
