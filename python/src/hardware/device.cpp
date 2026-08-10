// SPDX-License-Identifier: GPL-3.0-only

#include "device.hpp"

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/command_queue.hpp>
#include <xmipp4/core/hardware/event.hpp>
#include <xmipp4/core/hardware/event_usage_flags.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

class PyDevice : public device
{
public:
	using device::device;

	const memory_resource&
	get_memory_resource(memory_resource_affinity affinity) const override
	{
		PYBIND11_OVERRIDE_PURE(
			const memory_resource&,
			device,
			get_memory_resource,
			affinity
		);
	}

	std::shared_ptr<command_queue> create_command_queue() const override
	{
		PYBIND11_OVERRIDE_PURE(
			std::shared_ptr<command_queue>,
			device,
			create_command_queue,
		);
	}

	std::shared_ptr<event> create_event(event_usage_flags usage) const override
	{
		PYBIND11_OVERRIDE_PURE(
			std::shared_ptr<event>,
			device,
			create_event,
			usage
		);
	}
};

void bind_device(pybind11::module_ &m)
{
	py::class_<device, PyDevice, std::shared_ptr<device>>(m, "Device")
		.def(py::init<>())
		.def(
			"get_memory_resource",
			&device::get_memory_resource,
			py::arg("affinity"),
			py::return_value_policy::reference_internal
		)
		.def(
			"create_command_queue",
			&device::create_command_queue,
			py::keep_alive<0, 1>()
		)
		.def(
			"create_event",
			&device::create_event,
			py::arg("usage"),
			py::keep_alive<0, 1>()
		);
}

} // namespace hardware
} // namespace xmipp4
