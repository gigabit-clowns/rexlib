# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_creates_event(__setup_device):
	event = __setup_device.create_event(
		rexlib.hardware.EventUsageFlags(
			rexlib.hardware.EventUsageFlagBits.host_wait
		)
	)
	assert isinstance(event, rexlib.hardware.Event)

def test_event_reports_supported_usage(__setup_device):
	event = __setup_device.create_event(
		rexlib.hardware.EventUsageFlags(
			rexlib.hardware.EventUsageFlagBits.host_wait
		)
	)
	assert event.supported_usage.contains(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)

def test_event_is_signaled_after_being_signaled(__setup_session):
	event = __setup_session.device.create_event(
		rexlib.hardware.EventUsageFlags(
			rexlib.hardware.EventUsageFlagBits.host_query
		)
	)
	__setup_session.default_queue.signal(event)
	assert event.is_signaled is True

def test_waiting_on_a_signaled_event_returns(__setup_session):
	event = __setup_session.device.create_event(
		rexlib.hardware.EventUsageFlags(
			rexlib.hardware.EventUsageFlagBits.host_wait
		)
	)
	__setup_session.default_queue.signal(event)
	event.wait()

def test_queue_waits_on_event(__setup_session):
	event = __setup_session.device.create_event(
		rexlib.hardware.EventUsageFlags(
			rexlib.hardware.EventUsageFlagBits.device_wait
		)
	)
	__setup_session.default_queue.signal(event)
	__setup_session.default_queue.wait(event)

def test_flags_from_a_single_bit_contain_only_that_bit():
	flags = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	assert flags.contains(rexlib.hardware.EventUsageFlagBits.host_wait)
	assert not flags.contains(rexlib.hardware.EventUsageFlagBits.device_wait)
	assert flags.bits == rexlib.hardware.EventUsageFlagBits.host_wait.value

def test_flags_or_combines_bits():
	host_wait = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	device_wait = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.device_wait
	)
	combined = host_wait | device_wait
	assert combined.contains(rexlib.hardware.EventUsageFlagBits.host_wait)
	assert combined.contains(rexlib.hardware.EventUsageFlagBits.device_wait)

def test_flags_and_keeps_common_bits():
	host_wait = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	combined = host_wait | rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.device_wait
	)
	assert (combined & host_wait) == host_wait

def test_flags_xor_removes_common_bits():
	host_wait = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	assert (host_wait ^ host_wait).bits == 0

def test_flags_compare_equal_by_value():
	first = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	second = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.host_wait
	)
	other = rexlib.hardware.EventUsageFlags(
		rexlib.hardware.EventUsageFlagBits.device_wait
	)
	assert first == second
	assert first != other

def test_usage_flag_bits_are_distinct():
	names = ['host_query', 'host_wait', 'device_wait', 'cross_device_wait']
	bits = {getattr(rexlib.hardware.EventUsageFlagBits, name) for name in names}
	assert len(bits) == len(names)

def test_flags_can_be_built_from_an_integer():
	bits = rexlib.hardware.EventUsageFlagBits.host_wait.value
	assert rexlib.hardware.EventUsageFlags(bits).bits == bits

@pytest.fixture
def __setup_session():
	catalog = rexlib.ServiceCatalog()
	manager = rexlib.hardware.get_device_manager(catalog)
	return manager.create_device_session(rexlib.hardware.DeviceIndex('cpu', 0))

@pytest.fixture
def __setup_device(__setup_session):
	return __setup_session.device
