# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_returns_host_memory_resource():
	resource = rexlib.hardware.get_host_memory_resource()
	assert isinstance(resource, rexlib.hardware.MemoryResource)

def test_host_memory_resource_reports_its_kind():
	resource = rexlib.hardware.get_host_memory_resource()
	assert isinstance(resource.kind, rexlib.hardware.MemoryResourceKind)

def test_creates_allocator_from_memory_resource():
	resource = rexlib.hardware.get_host_memory_resource()
	allocator = resource.create_allocator()
	assert isinstance(allocator, rexlib.hardware.MemoryAllocator)

def test_allocator_refers_back_to_its_memory_resource():
	resource = rexlib.hardware.get_host_memory_resource()
	allocator = resource.create_allocator()
	assert allocator.memory_resource is resource

def test_allocator_reports_max_alignment():
	allocator = rexlib.hardware.get_host_memory_resource().create_allocator()
	assert allocator.max_alignment > 0

def test_device_provides_memory_resource_for_an_affinity(__setup_session):
	resource = __setup_session.device.get_memory_resource(
		rexlib.hardware.MemoryResourceAffinity.host
	)
	assert isinstance(resource, rexlib.hardware.MemoryResource)

def test_session_provides_allocator_for_an_affinity(__setup_session):
	allocator = __setup_session.get_allocator(
		rexlib.hardware.MemoryResourceAffinity.host
	)
	assert isinstance(allocator, rexlib.hardware.MemoryAllocator)

def test_memory_resource_kinds_are_distinct():
	names = [
		'device_local', 'device_mapped', 'host_staging', 'managed', 'unified'
	]
	kinds = {
		getattr(rexlib.hardware.MemoryResourceKind, name) for name in names
	}
	assert len(kinds) == len(names)

def test_memory_resource_kind_converts_to_string():
	assert 'managed' in str(rexlib.hardware.MemoryResourceKind.managed)

def test_memory_resource_affinity_converts_to_string():
	assert 'host' in str(rexlib.hardware.MemoryResourceAffinity.host)

@pytest.fixture
def __setup_session():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	return manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
