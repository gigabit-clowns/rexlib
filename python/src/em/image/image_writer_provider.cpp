// SPDX-License-Identifier: GPL-3.0-only

#include "image_writer_provider.hpp"

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <memory>
#include <string>

namespace rexlib
{

namespace py = pybind11;

static void py_declare(
	em::managed_image_writer_provider &self,
	const std::string &path,
	const em::image_descriptor &descriptor
)
{
	self.declare(path, descriptor, em::image_metadata());
}

image_writer_provider_class
declare_image_writer_provider(pybind11::module_ &m)
{
	return image_writer_provider_class(m, "ImageWriterProvider");
}

managed_image_writer_provider_class
declare_managed_image_writer_provider(pybind11::module_ &m)
{
	return managed_image_writer_provider_class(
		m,
		"ManagedImageWriterProvider"
	);
}

void define_managed_image_writer_provider(
	managed_image_writer_provider_class &c
)
{
	c
		.def(
			py::init<std::shared_ptr<const em::image_write_format_manager>>(),
			py::arg("formats")
		)
		.def(
			"declare", &py_declare,
			py::arg("path"), py::arg("descriptor")
		)
		.def(
			"close", &em::managed_image_writer_provider::close,
			py::arg("path"),
			py::call_guard<py::gil_scoped_release>()
		)
		.def(
			"flush", &em::managed_image_writer_provider::flush,
			py::call_guard<py::gil_scoped_release>()
		)
		.def_property_readonly(
			"file_count",
			&em::managed_image_writer_provider::get_file_count
		);
}

} // namespace rexlib
