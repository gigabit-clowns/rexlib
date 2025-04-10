#***************************************************************************
# Authors:     Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
#              Martín Salinas Antón (ssalinasmartin@gmail.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA
#
#  All comments concerning this program package may be sent to the
#  e-mail address 'xmipp@cnb.csic.es'
# ***************************************************************************
import pytest

import xmipp4

def test_returns_device(__setup_device_manager):
  assert __setup_device_manager.create_device(
    xmipp4.compute.DeviceIndex('host', 0)
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
def test_returns_expected_queues_for_host(__setup_device_manager, n_queues):
  device = __setup_device_manager.create_device(
    xmipp4.compute.DeviceIndex('host', 0),
    desired_queue_count=n_queues
  )
  queue_pool = device.get_queue_pool()
  assert len(queue_pool.queues) == 1

@pytest.fixture
def __setup_device_manager():
  ir = xmipp4.InterfaceCatalog()
  return xmipp4.compute.get_device_manager(ir)
