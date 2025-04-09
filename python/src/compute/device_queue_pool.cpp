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

#include "device_queue_pool.hpp"

#include <xmipp4/core/compute/device_queue.hpp>
#include <xmipp4/core/compute/device_queue_pool.hpp>

#include <sstream>

namespace xmipp4
{
namespace compute
{

namespace py = pybind11;

void bind_device_queue_pool(pybind11::module_ &m)
{
    py::class_<device_queue_pool>(m, "DeviceQueuePool")
        .def_property_readonly(
            "queues",
            [](py::object &self) -> py::list
            {
                py::list queues;

                auto &pool = self.cast<device_queue_pool&>();
                const auto size = pool.get_size();
                for (std::size_t i = 0; i < size; ++i)
                {
                    auto queue = py::cast(
                        pool.get_queue(i), 
                        py::return_value_policy::reference_internal,
                        self
                    );
                    queues.append(std::move(queue));
                }
                return queues;
            }
        );
}

} // namespace compute
} // namespace xmipp4
  