// SPDX-License-Identifier: GPL-3.0-only

#include "image_write.hpp"

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_saver.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <pybind11/stl.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static void py_write_single(
	const array &arr,
	const std::string &path,
	const em::image_write_format_manager &manager,
	std::optional<numerical_type> data_type
)
{
	em::write_single(
		arr,
		path,
		manager,
		data_type.value_or(numerical_type::unknown)
	);
}

static void py_write_stack(
	const array &arr,
	const std::string &path,
	const em::image_write_format_manager &manager,
	std::optional<numerical_type> data_type
)
{
	em::write_stack(
		arr,
		path,
		manager,
		data_type.value_or(numerical_type::unknown)
	);
}

static void py_write(
	const array &arr,
	const std::string &path,
	const em::image_write_format_manager &manager,
	const em::image_descriptor &descriptor
)
{
	em::write(arr, path, manager, descriptor);
}

// write_batch_async takes its source by value, so the caller's array is
// shared rather than moved out of.
static std::shared_ptr<completion> py_write_batch_async(
	const em::image_saver &saver,
	const array &source,
	const std::vector<em::image_location> &locations
)
{
	return em::write_batch_async(
		saver,
		source.share_const(),
		make_span(locations)
	);
}

void bind_image_write(pybind11::module_ &m)
{
	m.def(
		"write_single", &py_write_single,
		py::arg("array"), py::arg("path"), py::arg("manager"),
		py::arg("data_type") = py::none(),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"write_stack", &py_write_stack,
		py::arg("array"), py::arg("path"), py::arg("manager"),
		py::arg("data_type") = py::none(),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"write", &py_write,
		py::arg("array"), py::arg("path"), py::arg("manager"),
		py::arg("descriptor"),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"write_batch_async", &py_write_batch_async,
		py::arg("saver"), py::arg("source"), py::arg("locations"),
		py::call_guard<py::gil_scoped_release>()
	);
}

} // namespace rexlib
