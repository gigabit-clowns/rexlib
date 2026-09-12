# SPDX-License-Identifier: GPL-3.0-only

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
