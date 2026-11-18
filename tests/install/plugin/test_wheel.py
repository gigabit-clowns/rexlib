# SPDX-License-Identifier: GPL-3.0-only

import rexlib

def test_the_default_catalog_registers_the_installed_plugin():
	devices = rexlib.hardware.get_device_service(rexlib.get_default_catalog())

	assert "sample" in devices.backends

def test_an_array_is_allocated_on_the_device_of_the_installed_plugin():
	descriptor = rexlib.make_contiguous_array_descriptor(
		[5],
		rexlib.NumericalType.int32,
	)

	with rexlib.device("sample"):
		array = rexlib.empty(
			descriptor,
			rexlib.hardware.MemoryResourceAffinity.host,
		)

	assert isinstance(array, rexlib.Array)
