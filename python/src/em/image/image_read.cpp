// SPDX-License-Identifier: GPL-3.0-only

#include "image_read.hpp"

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/layout/index_table.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_loader.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <pybind11/stl.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static array py_read_path(
	const std::string &path,
	em::image_reader_provider &readers,
	const execution_context &context,
	std::optional<numerical_type> data_type
)
{
	return em::read(
		path,
		readers,
		context,
		data_type.value_or(numerical_type::unknown)
	);
}

static array py_read_location(
	const em::image_location &location,
	em::image_reader_provider &readers,
	const execution_context &context,
	std::optional<numerical_type> data_type
)
{
	return em::read(
		location,
		readers,
		context,
		data_type.value_or(numerical_type::unknown)
	);
}

// array is move-only and the asynchronous reads take one by value, so the
// caller's array is shared rather than moved out of: a moved-from Array
// would be left empty in their hands.
static std::shared_ptr<completion> py_read_batch_async(
	const em::image_loader &loader,
	array &destination,
	const std::vector<em::image_location> &locations
)
{
	return em::read_batch_async(
		loader,
		destination.share(),
		make_span(locations)
	);
}

static std::shared_ptr<completion> py_read_patches_async(
	const em::image_loader &loader,
	array &destination,
	const em::image_location &location,
	const index_table &centres
)
{
	return em::read_patches_async(
		loader,
		destination.share(),
		location,
		centres
	);
}

void bind_image_read(pybind11::module_ &m)
{
	m.def(
		"read", &py_read_path,
		py::arg("path"), py::arg("readers"), py::arg("context"),
		py::arg("data_type") = py::none(),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"read", &py_read_location,
		py::arg("location"), py::arg("readers"), py::arg("context"),
		py::arg("data_type") = py::none(),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"read_batch_async", &py_read_batch_async,
		py::arg("loader"), py::arg("destination"), py::arg("locations"),
		py::call_guard<py::gil_scoped_release>()
	);
	m.def(
		"read_patches_async", &py_read_patches_async,
		py::arg("loader"), py::arg("destination"), py::arg("location"),
		py::arg("centres"),
		py::call_guard<py::gil_scoped_release>()
	);
}

} // namespace rexlib
