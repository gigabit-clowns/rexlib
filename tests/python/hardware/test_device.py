# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_returns_device(__setup_device_manager):
  assert __setup_device_manager.create_device(
    xmipp4.hardware.DeviceIndex('cpu', 0)
  ) is not None

@pytest.fixture
def __setup_device_manager():
  ir = xmipp4.ServiceCatalog()
  return xmipp4.hardware.get_device_manager(ir)
