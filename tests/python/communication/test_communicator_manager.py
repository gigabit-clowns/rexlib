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
