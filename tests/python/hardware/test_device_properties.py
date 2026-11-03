# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_manager_returns_device_properties(__setup_device_manager):
	properties = __setup_device_manager.get_device_properties(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)
	assert isinstance(properties, rexlib.hardware.DeviceProperties)

def test_session_exposes_device_properties(__setup_session):
	assert isinstance(
		__setup_session.properties, rexlib.hardware.DeviceProperties
	)

def test_cpu_device_reports_cpu_type(__setup_properties):
	assert __setup_properties.type == rexlib.hardware.DeviceType.CPU

def test_device_reports_a_name(__setup_properties):
	assert isinstance(__setup_properties.name, str)

def test_device_reports_a_physical_location(__setup_properties):
	assert isinstance(__setup_properties.physical_location, str)

def test_device_reports_total_memory(__setup_properties):
	assert __setup_properties.total_memory_bytes > 0

def test_device_reports_optimal_data_alignment(__setup_properties):
	assert __setup_properties.optimal_data_alignment > 0

def test_device_properties_are_writable():
	properties = rexlib.hardware.DeviceProperties()
	properties.name = 'mock'
	properties.type = rexlib.hardware.DeviceType.GPU
	properties.total_memory_bytes = 1024
	properties.optimal_data_alignment = 64
	assert properties.name == 'mock'
	assert properties.type == rexlib.hardware.DeviceType.GPU
	assert properties.total_memory_bytes == 1024
	assert properties.optimal_data_alignment == 64

def test_device_types_are_distinct():
	names = ['unknown', 'CPU', 'GPU', 'iGPU']
	types = {getattr(rexlib.hardware.DeviceType, name) for name in names}
	assert len(types) == len(names)

def test_device_type_converts_to_string():
	assert 'CPU' in str(rexlib.hardware.DeviceType.CPU)

def test_manager_returns_backend_by_name(__setup_device_manager):
	backend = __setup_device_manager.get_backend('cpu')
	assert isinstance(backend, rexlib.hardware.DeviceBackend)

def test_backend_reports_name_and_version(__setup_backend):
	assert __setup_backend.name == 'cpu'
	assert isinstance(__setup_backend.version, rexlib.Version)

def test_backend_lists_its_devices(__setup_backend):
	assert __setup_backend.devices == [0]

def test_backend_returns_device_properties(__setup_backend):
	properties = __setup_backend.get_device_properties(0)
	assert properties.type == rexlib.hardware.DeviceType.CPU

def test_backend_creates_a_device(__setup_backend):
	assert isinstance(
		__setup_backend.create_device(0), rexlib.hardware.Device
	)

@pytest.fixture
def __setup_device_manager():
	catalog = rexlib.ServiceCatalog()
	return rexlib.hardware.get_device_manager(catalog)

@pytest.fixture
def __setup_session(__setup_device_manager):
	return __setup_device_manager.create_device_session(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)

@pytest.fixture
def __setup_properties(__setup_device_manager):
	return __setup_device_manager.get_device_properties(
		rexlib.hardware.DeviceIndex('cpu', 0)
	)

@pytest.fixture
def __setup_backend(__setup_device_manager):
	return __setup_device_manager.get_backend('cpu')
