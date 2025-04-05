/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#include "device.hpp"

#include <xmipp4/core/compute/device.hpp>
#include <xmipp4/core/compute/device_queue_pool.hpp>
#include <xmipp4/core/compute/device_memory_allocator.hpp>
#include <xmipp4/core/compute/host_memory_allocator.hpp>
#include <xmipp4/core/compute/host_to_device_transfer.hpp>
#include <xmipp4/core/compute/device_to_host_transfer.hpp>
#include <xmipp4/core/compute/device_copy.hpp>
#include <xmipp4/core/compute/device_event.hpp>
#include <xmipp4/core/compute/device_to_host_event.hpp>

namespace xmipp4
{
namespace compute
{

namespace py = pybind11;

void bind_device(pybind11::module_ &m)
{
    py::class_<device, std::shared_ptr<device>>(m, "Device")
        .def(
            "get_queue_pool",
            &device::get_queue_pool,
            py::return_value_policy::reference,
            py::keep_alive<0, 1>()
        )
        .def("create_device_memory_allocator", &device::create_device_memory_allocator)
        .def("create_host_memory_allocator", &device::create_host_memory_allocator)
        .def("create_host_to_device_transfer", &device::create_host_to_device_transfer)
        .def("create_device_to_host_transfer", &device::create_device_to_host_transfer)
        .def("create_device_copy", &device::create_device_copy)
        .def("create_device_event", &device::create_device_event)
        .def("create_device_to_host_event", &device::create_device_to_host_event);

}

} // namespace compute
} // namespace xmipp4
