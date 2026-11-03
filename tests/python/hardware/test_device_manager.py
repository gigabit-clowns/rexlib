# SPDX-License-Identifier: GPL-3.0-only

import pytest

import rexlib

def test_returns_device_manager(__setup_service_catalog):
  assert isinstance(
    rexlib.hardware.get_device_manager(__setup_service_catalog),
    rexlib.hardware.DeviceManager
  )

def test_always_returns_same_device_manager(__setup_service_catalog):
  dm1 = rexlib.hardware.get_device_manager(__setup_service_catalog)
  dm2 = rexlib.hardware.get_device_manager(__setup_service_catalog)
  assert dm1 is dm2

def test_returns_default_backends(__setup_service_catalog):
  dm = rexlib.hardware.get_device_manager(
    __setup_service_catalog
  )
  assert dm.backends == ['cpu']

def test_returns_default_devices(__setup_service_catalog):
  dm = rexlib.hardware.get_device_manager(
    __setup_service_catalog
  )
  assert dm.devices == [rexlib.hardware.DeviceIndex('cpu', 0)]

@pytest.fixture
def __setup_service_catalog():
  return rexlib.ServiceCatalog()
