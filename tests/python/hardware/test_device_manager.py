# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_returns_device_manager(__setup_service_catalog):
  assert xmipp4.hardware.get_device_manager(
    __setup_service_catalog
  ) is not None

def test_always_returns_same_device_manager(__setup_service_catalog):
  dm1 = xmipp4.hardware.get_device_manager(__setup_service_catalog)
  dm2 = xmipp4.hardware.get_device_manager(__setup_service_catalog)
  assert dm1 is dm2

def test_returns_default_backends(__setup_service_catalog):
  dm = xmipp4.hardware.get_device_manager(
    __setup_service_catalog
  )
  assert dm.backends == ['cpu']

def test_returns_default_devices(__setup_service_catalog):
  dm = xmipp4.hardware.get_device_manager(
    __setup_service_catalog
  )
  assert dm.devices == [xmipp4.hardware.DeviceIndex('cpu', 0)]

@pytest.fixture
def __setup_service_catalog():
  return xmipp4.ServiceCatalog()
