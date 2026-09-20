// SPDX-License-Identifier: GPL-3.0-only

#include "completion.hpp"

namespace rexlib
{

namespace py = pybind11;

completion_class declare_completion(pybind11::module_ &m)
{
	return completion_class(m, "Completion");
}

void define_completion(completion_class &c)
{
	c
		.def(
			"wait", &completion::wait,
			py::call_guard<py::gil_scoped_release>()
		)
		.def(
			"get", &completion::get,
			py::call_guard<py::gil_scoped_release>()
		)
		.def_property_readonly("is_ready", &completion::is_ready);
}

} // namespace rexlib
