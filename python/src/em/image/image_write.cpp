// SPDX-License-Identifier: GPL-3.0-only

#include "image_write.hpp"

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <pybind11/stl.h>

#include <optional>
#include <string>

namespace rexlib
{

namespace py = pybind11;

static void py_write(
	const array &arr,
	const std::string &path,
	const em::image_write_format_manager &manager,
	std::optional<numerical_type> data_type
)
{
	em::write(
		arr,
		path,
		manager,
		data_type.value_or(numerical_type::unknown)
	);
}

void bind_image_write(pybind11::module_ &m)
{
	m.def(
		"write", &py_write,
		py::arg("array"), py::arg("path"), py::arg("manager"),
		py::arg("data_type") = py::none()
	);
}

} // namespace rexlib
