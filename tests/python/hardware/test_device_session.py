# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_creates_session_for_cpu_device(__setup_device_manager):
	session = __setup_device_manager.create_device_session(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)
	assert isinstance(session, rexlib.hardware.DeviceSession)
	assert isinstance(session.device, rexlib.hardware.Device)
	assert isinstance(session.default_queue, rexlib.hardware.CommandQueue)

def test_session_provides_host_allocator(__setup_device_manager):
	session = __setup_device_manager.create_device_session(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)
	allocator = session.get_allocator(rexlib.hardware.MemoryResourceAffinity.host)
	assert allocator is not None

@pytest.fixture
def __setup_device_manager():
	catalog = rexlib.ServiceCatalog()
	return rexlib.hardware.get_device_manager(catalog)
