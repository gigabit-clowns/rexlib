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

def test_returns_device_manager(__setup_interface_registry):
  assert xmipp4.compute.get_device_manager(
    __setup_interface_registry
  ) is not None

def test_always_returns_same_device_manager(__setup_interface_registry):
  dm1 = xmipp4.compute.get_device_manager(__setup_interface_registry)
  dm2 = xmipp4.compute.get_device_manager(__setup_interface_registry)
  assert dm1 is dm2

def test_returns_default_backends(__setup_interface_registry):
  dm = xmipp4.compute.get_device_manager(
    __setup_interface_registry
  )
  assert dm.backends == ['host']

def test_returns_default_devices(__setup_interface_registry):
  dm = xmipp4.compute.get_device_manager(
    __setup_interface_registry
  )
  assert dm.devices == [xmipp4.compute.DeviceIndex('host', 0)]

@pytest.fixture
def __setup_interface_registry():
  return xmipp4.InterfaceRegistry()
