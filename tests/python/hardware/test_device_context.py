# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_empty_context_has_no_session():
	ctx = xmipp4.hardware.DeviceContext()
	assert ctx.device_session is None

def test_context_wraps_session(__setup_session):
	ctx = xmipp4.hardware.DeviceContext(__setup_session)
	assert ctx.device_session is __setup_session
	assert ctx.active_queue is __setup_session.default_queue

def test_on_queue_returns_updated_copy(__setup_session):
	ctx = xmipp4.hardware.DeviceContext(__setup_session)
	other_queue = __setup_session.device.create_command_queue()
	updated = ctx.on_queue(other_queue)
	assert updated.active_queue is other_queue
	assert ctx.active_queue is __setup_session.default_queue

@pytest.fixture
def __setup_session():
	catalog = xmipp4.ServiceCatalog()
	manager = xmipp4.hardware.get_device_manager(catalog)
	return manager.create_device_session(xmipp4.hardware.DeviceIndex('cpu', 0))
