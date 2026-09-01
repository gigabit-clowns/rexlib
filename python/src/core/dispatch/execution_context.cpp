// SPDX-License-Identifier: GPL-3.0-only

#include "execution_context.hpp"

#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/dispatch/dispatcher.hpp>
#include <rexlib/core/hardware/device_context.hpp>

namespace rexlib
{

namespace py = pybind11;

execution_context_class declare_execution_context(pybind11::module_ &m)
{
	return execution_context_class(m, "ExecutionContext");
}

void define_execution_context(execution_context_class &c)
{
	c
		.def(py::init<>())
		.def(
			py::init<device_context, std::shared_ptr<dispatcher>>(),
			py::arg("device_context"), py::arg("dispatcher")
		)
		.def_property_readonly(
			"device_context",
			&execution_context::get_device_context
		)
		.def_property_readonly(
			"dispatcher",
			&execution_context::get_dispatcher
		)
		.def(
			"with_device_context",
			&execution_context::with_device_context,
			py::arg("device_context")
		)
		.def(
			"with_dispatcher",
			&execution_context::with_dispatcher,
			py::arg("dispatcher")
		);

	}
} // namespace rexlib
