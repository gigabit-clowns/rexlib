// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/device_queue.hpp>
#include <xmipp4/core/hardware/device_event.hpp>
#include <xmipp4/core/hardware/device_to_host_event.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

class PyDevice : public device
{
	public:
    using device::device;

		void enumerate_memory_resources(
			std::vector<memory_resource*> &
    ) override
		{
			PYBIND11_OVERRIDE_PURE(
				void,
				device,
				enumerate_memory_resources,
			);
		}
	
		std::shared_ptr<device_queue> create_device_queue() override
		{
			PYBIND11_OVERRIDE_PURE(
				std::shared_ptr<device_queue>,
				device,
				create_device_queue,
			);
		}

		std::shared_ptr<device_event> create_device_event() override
		{
			PYBIND11_OVERRIDE_PURE(
				std::shared_ptr<device_event>,
				device,
				create_device_event,
			);
		}

		std::shared_ptr<device_to_host_event> create_device_to_host_event() override
		{
			PYBIND11_OVERRIDE_PURE(
				std::shared_ptr<device_to_host_event>,
				device,
				create_device_to_host_event,
			);
		}
};

void bind_device(pybind11::module_ &m)
{
	py::class_<device, PyDevice, std::shared_ptr<device>>(m, "Device")
		.def(py::init<>())
		.def(
			"create_device_queue", 
			&device::create_device_queue,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_device_event",
			&device::create_device_event,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_device_to_host_event", 
			&device::create_device_to_host_event,
			py::keep_alive<0, 1>()
		);
}

} // namespace hardware
} // namespace xmipp4
