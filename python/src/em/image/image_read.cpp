// SPDX-License-Identifier: GPL-3.0-only

#include "image_read.hpp"

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>

#include <string>

namespace rexlib
{

namespace py = pybind11;

void bind_image_read(pybind11::module_ &m)
{
	m.def(
		"read",
		py::overload_cast<
			const std::string&,
			const em::image_read_format_manager&,
			const execution_context&
		>(&em::read),
		py::arg("path"), py::arg("manager"), py::arg("context")
	);
	m.def(
		"read",
		py::overload_cast<
			const em::image_location&,
			const em::image_read_format_manager&,
			const execution_context&
		>(&em::read),
		py::arg("location"), py::arg("manager"), py::arg("context")
	);
}

} // namespace rexlib
