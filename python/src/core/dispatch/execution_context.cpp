// SPDX-License-Identifier: GPL-3.0-only

#include "execution_context.hpp"

#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/hardware/device_context.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

execution_context_class declare_execution_context(nb::module_ &m)
{
	return execution_context_class(m, "ExecutionContext");
}

void define_execution_context(execution_context_class &c)
{
	c
		.def(nb::init<>())
		.def(
			nb::init<device_context, std::shared_ptr<dispatcher>>(),
			nb::arg("device_context"), nb::arg("dispatcher")
		)
		.def_prop_ro(
			"device_context",
			&execution_context::get_device_context
		)
		.def_prop_ro(
			"dispatcher",
			&execution_context::get_dispatcher
		)
		.def(
			"with_device_context",
			&execution_context::with_device_context,
			nb::arg("device_context")
		)
		.def(
			"with_dispatcher",
			&execution_context::with_dispatcher,
			nb::arg("dispatcher")
		);

	}
} // namespace rexlib
