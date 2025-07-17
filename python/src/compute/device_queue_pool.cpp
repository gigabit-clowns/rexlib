// SPDX-License-Identifier: GPL-3.0-only

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
  