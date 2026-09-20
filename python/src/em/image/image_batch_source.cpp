// SPDX-License-Identifier: GPL-3.0-only

#include "image_batch_source.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/executor.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <pybind11/stl.h>

#include <memory>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

// array is move-only and read takes one by value, so the caller's array is
// shared rather than moved out of: a moved-from Array would be left empty in
// their hands.
static std::shared_ptr<completion> py_read(
	const em::image_batch_source &self,
	array &destination,
	const std::vector<em::image_location> &locations
)
{
	return self.read(destination.share(), make_span(locations));
}

image_source_class declare_image_source(pybind11::module_ &m)
{
	return image_source_class(m, "ImageSource");
}

image_batch_source_class declare_image_batch_source(pybind11::module_ &m)
{
	return image_batch_source_class(m, "ImageBatchSource");
}

void define_image_source(image_source_class &c)
{
	c.def(
		py::init<
			std::shared_ptr<em::image_reader_provider>,
			std::shared_ptr<executor>
		>(),
		py::arg("readers"), py::arg("executor")
	);
}

void define_image_batch_source(image_batch_source_class &c)
{
	c
		.def(
			py::init<std::shared_ptr<const em::image_source>>(),
			py::arg("source")
		)
		.def(
			"read", &py_read,
			py::arg("destination"), py::arg("locations"),
			py::call_guard<py::gil_scoped_release>()
		);
}

} // namespace rexlib
