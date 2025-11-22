# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_should_allow_inheriting_from_device(__setup_device):
	assert isinstance(__setup_device, xmipp4.hardware.Device)

@pytest.fixture
def __setup_device():
	class DeviceMock(xmipp4.hardware.Device):
		pass
	

	return DeviceMock()
