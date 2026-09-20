# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_default_descriptor_is_not_initialized():
	descriptor = rexlib.ArrayDescriptor()
	assert not rexlib.is_initialized(descriptor)

def test_contiguous_descriptor_has_requested_data_type():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.float32
	)
	assert rexlib.is_initialized(descriptor)
	assert descriptor.data_type == rexlib.NumericalType.float32

def test_equal_descriptors_compare_equal():
	d1 = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	d2 = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	assert d1 == d2

def test_different_descriptors_compare_not_equal():
	d1 = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	d2 = rexlib.make_contiguous_array_descriptor(
		[3, 2], rexlib.NumericalType.int32
	)
	assert d1 != d2

def test_reports_the_shape_it_was_made_with():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3, 4], rexlib.NumericalType.float32
	)
	assert descriptor.shape == (2, 3, 4)

def test_default_descriptor_has_no_shape():
	assert rexlib.ArrayDescriptor().shape == ()

def test_equal_descriptors_hash_equal():
	d1 = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	d2 = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	assert hash(d1) == hash(d2)

def test_usable_as_a_dictionary_key():
	table = {
		rexlib.make_contiguous_array_descriptor(
			[2, 3], rexlib.NumericalType.int32
		): 'value'
	}
	key = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	assert table[key] == 'value'

def test_repr_names_the_type():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.int32
	)
	assert repr(descriptor).startswith('ArrayDescriptor(')

def test_repr_of_a_default_descriptor():
	assert repr(rexlib.ArrayDescriptor()) == 'ArrayDescriptor()'

def test_shape_is_a_tuple():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.float32
	)
	assert isinstance(descriptor.shape, tuple)

@pytest.mark.parametrize(
	"extents",
	[
		pytest.param([2, 3], id="From a list"),
		pytest.param((2, 3), id="From a tuple"),
		pytest.param(range(2, 4), id="From a range"),
	]
)
def test_takes_any_sequence_of_extents(extents):
	descriptor = rexlib.make_contiguous_array_descriptor(
		extents, rexlib.NumericalType.float32
	)
	assert descriptor.shape == (2, 3)

def test_the_shape_it_reports_is_one_it_accepts():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 3], rexlib.NumericalType.float32
	)
	assert rexlib.make_contiguous_array_descriptor(
		descriptor.shape, rexlib.NumericalType.float32
	) == descriptor
