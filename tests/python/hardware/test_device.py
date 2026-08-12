# SPDX-License-Identifier: GPL-3.0-only

import gc

import pytest

import xmipp4

class DeviceMock(xmipp4.hardware.Device):
	"""A Device implemented in Python, recording what C++ asks of it."""

	def __init__(self, queue):
		super().__init__()
		self.queue = queue
		self.calls = []

	def get_memory_resource(self, affinity):
		self.calls.append(('get_memory_resource', affinity))
		return xmipp4.hardware.get_host_memory_resource()

	def create_command_queue(self):
		self.calls.append(('create_command_queue',))
		return self.queue

	def create_event(self, usage):
		self.calls.append(('create_event', usage))

def test_should_allow_inheriting_from_device(__setup_mock_device):
	assert isinstance(__setup_mock_device, xmipp4.hardware.Device)

def test_building_a_session_calls_back_into_python(
	__setup_mock_device, __setup_properties
):
	xmipp4.hardware.DeviceSession(__setup_mock_device, __setup_properties)
	assert ('create_command_queue',) in __setup_mock_device.calls

def test_session_queries_a_memory_resource_for_every_affinity(
	__setup_mock_device, __setup_properties
):
	xmipp4.hardware.DeviceSession(__setup_mock_device, __setup_properties)
	queried = {
		call[1] for call in __setup_mock_device.calls
		if call[0] == 'get_memory_resource'
	}
	assert queried == {
		xmipp4.hardware.MemoryResourceAffinity.host,
		xmipp4.hardware.MemoryResourceAffinity.device,
	}

def test_session_wraps_the_python_device(
	__setup_mock_device, __setup_properties
):
	session = xmipp4.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	assert session.device is __setup_mock_device
	assert session.properties.name == __setup_properties.name

def test_session_keeps_the_python_device_alive(__setup_queue, __setup_properties):
	session = xmipp4.hardware.DeviceSession(
		DeviceMock(__setup_queue), __setup_properties
	)
	gc.collect()
	assert session.get_allocator(
		xmipp4.hardware.MemoryResourceAffinity.host
	) is not None

def test_missing_override_raises_when_called_from_cpp(__setup_properties):
	class Bare(xmipp4.hardware.Device):
		pass

	bare = Bare()
	with pytest.raises(RuntimeError, match='pure virtual'):
		xmipp4.hardware.DeviceSession(bare, __setup_properties)

def test_python_device_is_usable_as_an_execution_context(
	__setup_mock_device, __setup_properties
):
	session = xmipp4.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	with xmipp4.device(session) as context:
		assert context.device_context.device_session is session

def test_arrays_can_be_created_on_a_python_device(
	__setup_mock_device, __setup_properties
):
	session = xmipp4.hardware.DeviceSession(
		__setup_mock_device, __setup_properties
	)
	descriptor = xmipp4.ndarray.make_contiguous_array_descriptor(
		[2, 2], xmipp4.numerical.NumericalType.float32
	)
	with xmipp4.device(session):
		result = xmipp4.functional.zeros(
			descriptor, xmipp4.hardware.MemoryResourceAffinity.host
		)
	assert isinstance(result, xmipp4.ndarray.Array)

@pytest.fixture
def __setup_queue():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	session = manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
	return session.default_queue

@pytest.fixture
def __setup_mock_device(__setup_queue):
	return DeviceMock(__setup_queue)

@pytest.fixture
def __setup_properties():
	properties = xmipp4.hardware.DeviceProperties()
	properties.name = 'mock'
	properties.type = xmipp4.hardware.DeviceType.CPU
	properties.total_memory_bytes = 2048
	properties.optimal_data_alignment = 128
	return properties
