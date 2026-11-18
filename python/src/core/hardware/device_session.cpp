// SPDX-License-Identifier: GPL-3.0-only

#include "device_session.hpp"

#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/hardware/command_queue.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

device_session_class declare_device_session(nb::module_ &m)
{
	return device_session_class(m, "DeviceSession");
}

void define_device_session(device_session_class &c)
{
	c
		.def(
			nb::init<std::shared_ptr<device>, device_properties>(),
			nb::arg("device"), nb::arg("properties")
		)
		.def_prop_ro("device", &device_session::get_device)
		.def_prop_ro(
			"properties",
			&device_session::get_properties,
			nb::rv_policy::reference_internal
		)
		.def(
			"get_allocator",
			&device_session::get_allocator,
			nb::arg("affinity")
		)
		.def_prop_ro(
			"default_queue",
			&device_session::get_default_queue
		);
}

} // namespace rexlib
