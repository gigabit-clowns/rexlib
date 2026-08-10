// SPDX-License-Identifier: GPL-3.0-only

#include "execution_context.hpp"

#include <xmipp4/core/dispatch/execution_context.hpp>
#include <xmipp4/core/dispatch/dispatcher.hpp>
#include <xmipp4/core/hardware/device_context.hpp>

namespace xmipp4
{
namespace dispatch
{

namespace py = pybind11;

void bind_execution_context(pybind11::module_ &m)
{
	py::class_<execution_context>(m, "ExecutionContext")
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

} // namespace dispatch
} // namespace xmipp4
