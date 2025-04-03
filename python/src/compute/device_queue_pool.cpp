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

#include <xmipp4/core/compute/device_queue_pool.hpp>
#include <xmipp4/core/compute/device_queue.hpp>

#include <sstream>

namespace xmipp4
{
namespace compute
{

namespace py = pybind11;

static std::string to_repr(const device_queue_pool &l)
{
    std::ostringstream oss;
    oss << "DeviceQueuePool(size=\"" << l.get_size() << ")";
    return oss.str();
}



void bind_device_queue_pool(pybind11::module_ &m)
{
    py::class_<device_queue_pool>(m, "DeviceQueuePool")
        .def("__repr__", &to_repr)
        .def(
            "get_size",
            [](const device_queue_pool &self) -> size_t
            {
            return self.get_size();
            }
        )
        .def(
            "get_queue",
            [](const device_queue_pool &self, const device_index &index) -> std::shared_ptr<device_queue>
            {
                std::shared_ptr<device_queue> desc = self.get_queue(index);
                if(!desc) {
                    throw std::invalid_argument("Requested device queue does not exist.");
                }
                return desc;
            }
        );

}

} // namespace compute
} // namespace xmipp4
  