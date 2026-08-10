# SPDX-License-Identifier: GPL-3.0-only

import xmipp4

def test_default_descriptor_is_not_initialized():
	descriptor = xmipp4.ndarray.ArrayDescriptor()
	assert not xmipp4.ndarray.is_initialized(descriptor)

def test_contiguous_descriptor_has_requested_data_type():
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.float32
	)
	assert xmipp4.ndarray.is_initialized(descriptor)
	assert descriptor.data_type == xmipp4.numerical.NumericalType.float32

def test_equal_descriptors_compare_equal():
	d1 = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.int32
	)
	d2 = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.int32
	)
	assert d1 == d2

def test_different_descriptors_compare_not_equal():
	d1 = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 3], xmipp4.numerical.NumericalType.int32
	)
	d2 = xmipp4.ndarray.make_contiguous_array_descriptor(
		[3, 2], xmipp4.numerical.NumericalType.int32
	)
	assert d1 != d2
