// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/command_queue.hpp>
#include <xmipp4/core/hardware/event.hpp>
#include <xmipp4/core/hardware/event_usage_flags.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

/**
 * Lets device be implemented in Python: each override forwards the call
 * back to the Python object. Defined here rather than in the source file
 * because it is part of device_class, which callers need to see.
 */
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

using device_class = pybind11::class_<device, PyDevice, std::shared_ptr<device>>;

device_class declare_device(pybind11::module_ &m);
void define_device(device_class &c);

} // namespace hardware
} // namespace xmipp4
