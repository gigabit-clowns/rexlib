// SPDX-License-Identifier: GPL-3.0-only

#include "device_context.hpp"

#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

device_context_class declare_device_context(nb::module_ &m)
{
	return device_context_class(m, "DeviceContext");
}

void define_device_context(device_context_class &c)
{
	c
		.def(nb::init<>())
		.def(
			nb::init<std::shared_ptr<const device_session>>(),
			nb::arg("session")
		)
		.def_prop_ro(
			"device_session",
			&device_context::get_device_session
		)
		.def_prop_ro(
			"active_queue",
			&device_context::get_active_queue
		)
		.def(
			"get_allocator",
			&device_context::get_allocator,
			nb::arg("affinity")
		)
		.def(
			"on_queue",
			&device_context::on_queue,
			nb::arg("queue")
		)
		.def(
			"with_allocator",
			&device_context::with_allocator,
			nb::arg("affinity"), nb::arg("allocator")
		);
}

} // namespace rexlib
