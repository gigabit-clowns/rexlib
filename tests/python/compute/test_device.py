# SPDX-License-Identifier: GPL-3.0-only

import pytest

import xmipp4

def test_returns_device(__setup_device_manager):
  assert __setup_device_manager.create_device(
    xmipp4.compute.DeviceIndex('cpu', 0)
  ) is not None

@pytest.mark.parametrize(
  'n_queues',
  [
    pytest.param(0, id='Zero queues requested'),
    pytest.param(1, id='One queue requested'),
    pytest.param(2, id='Two queues requested'),
    pytest.param(3, id='Three queues requested')
  ]
)
def test_returns_expected_queues_for_cpu(__setup_device_manager, n_queues):
  device = __setup_device_manager.create_device(
    xmipp4.compute.DeviceIndex('cpu', 0),
    desired_queue_count=n_queues
  )
  queue_pool = device.get_queue_pool()
  assert len(queue_pool.queues) == 1

@pytest.fixture
def __setup_device_manager():
  ir = xmipp4.InterfaceCatalog()
  return xmipp4.compute.get_device_manager(ir)
