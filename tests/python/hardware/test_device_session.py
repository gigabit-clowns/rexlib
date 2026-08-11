# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_creates_session_for_cpu_device(__setup_device_manager):
	session = __setup_device_manager.create_device_session(
		xmipp4.hardware.DeviceIndex('cpu', 0)
	)
	assert isinstance(session, xmipp4.hardware.DeviceSession)
	assert isinstance(session.device, xmipp4.hardware.Device)
	assert isinstance(session.default_queue, xmipp4.hardware.CommandQueue)

def test_session_provides_host_allocator(__setup_device_manager):
	session = __setup_device_manager.create_device_session(
		xmipp4.hardware.DeviceIndex('cpu', 0)
	)
	allocator = session.get_allocator(xmipp4.hardware.MemoryResourceAffinity.host)
	assert allocator is not None

@pytest.fixture
def __setup_device_manager():
	catalog = xmipp4.ServiceCatalog()
	return xmipp4.hardware.get_device_manager(catalog)
