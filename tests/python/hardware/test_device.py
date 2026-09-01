# SPDX-License-Identifier: GPL-3.0-only

import gc

import pytest

import rexlib

class DeviceMock(rexlib.hardware.Device):
	"""A Device implemented in Python, recording what C++ asks of it."""

	def __init__(self, queue):
		super().__init__()
		self.queue = queue
		self.calls = []

	def get_memory_resource(self, affinity):
		self.calls.append(('get_memory_resource', affinity))
		return rexlib.hardware.get_host_memory_resource()

	def create_command_queue(self):
		self.calls.append(('create_command_queue',))
		return self.queue

	def create_event(self, usage):
		self.calls.append(('create_event', usage))

def test_should_allow_inheriting_from_device(__setup_mock_device):
	assert isinstance(__setup_mock_device, rexlib.hardware.Device)

def test_building_a_session_calls_back_into_python(
	__setup_mock_device, __setup_properties
):
	rexlib.hardware.DeviceSession(__setup_mock_device, __setup_properties)
	assert ('create_command_queue',) in __setup_mock_device.calls

def test_session_queries_a_memory_resource_for_every_affinity(
	__setup_mock_device, __setup_properties
):
	rexlib.hardware.DeviceSession(__setup_mock_device, __setup_properties)
	queried = {
		call[1] for call in __setup_mock_device.calls
		if call[0] == 'get_memory_resource'
	}
	assert queried == {
		rexlib.hardware.MemoryResourceAffinity.host,
		rexlib.hardware.MemoryResourceAffinity.device,
	}

def test_session_wraps_the_python_device(
	__setup_mock_device, __setup_properties
):
	session = rexlib.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	assert session.device is __setup_mock_device
	assert session.properties.name == __setup_properties.name

def test_session_keeps_the_python_device_alive(__setup_queue, __setup_properties):
	session = rexlib.hardware.DeviceSession(
		DeviceMock(__setup_queue), __setup_properties
	)
	gc.collect()
	assert session.get_allocator(
		rexlib.hardware.MemoryResourceAffinity.host
	) is not None

def test_missing_override_raises_when_called_from_cpp(__setup_properties):
	class Bare(rexlib.hardware.Device):
		pass

	bare = Bare()
	with pytest.raises(RuntimeError, match='pure virtual'):
		rexlib.hardware.DeviceSession(bare, __setup_properties)

def test_python_device_is_usable_as_an_execution_context(
	__setup_mock_device, __setup_properties
):
	session = rexlib.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	with rexlib.device(session) as context:
		assert context.device_context.device_session is session

def test_arrays_can_be_created_on_a_python_device(
	__setup_mock_device, __setup_properties
):
	session = rexlib.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	descriptor = rexlib.make_contiguous_array_descriptor(
		[2, 2], rexlib.NumericalType.float32
	)
	with rexlib.device(session):
		result = rexlib.zeros(
			descriptor, rexlib.hardware.MemoryResourceAffinity.host
		)
	assert isinstance(result, rexlib.Array)

@pytest.fixture
def __setup_queue():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	session = manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))
	return session.default_queue

@pytest.fixture
def __setup_mock_device(__setup_queue):
	return DeviceMock(__setup_queue)

@pytest.fixture
def __setup_properties():
	properties = rexlib.hardware.DeviceProperties()
	properties.name = 'mock'
	properties.type = rexlib.hardware.DeviceType.CPU
	properties.total_memory_bytes = 2048
	properties.optimal_data_alignment = 128
	return properties
