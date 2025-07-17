# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_returns_communicator_manager(__setup_interface_catalog):
  assert xmipp4.communication.get_communicator_manager(
    __setup_interface_catalog
  ) is not None

def test_always_returns_same_communicator_manager(__setup_interface_catalog):
  cm1 = xmipp4.communication.get_communicator_manager(__setup_interface_catalog)
  cm2 = xmipp4.communication.get_communicator_manager(__setup_interface_catalog)
  assert cm1 is cm2

def test_returns_default_backends(__setup_interface_catalog):
  cm = xmipp4.communication.get_communicator_manager(
    __setup_interface_catalog
  )
  assert cm.backends == ['dummy']

@pytest.fixture
def __setup_interface_catalog():
  return xmipp4.InterfaceCatalog()
