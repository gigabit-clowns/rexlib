// SPDX-License-Identifier: GPL-3.0-only

#include "image_reader_provider.hpp"

#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>

#include <cstddef>
#include <memory>
#include <string>

namespace rexlib
{

namespace py = pybind11;

image_reader_provider_class
declare_image_reader_provider(pybind11::module_ &m)
{
	return image_reader_provider_class(m, "ImageReaderProvider");
}

direct_image_reader_provider_class
declare_direct_image_reader_provider(pybind11::module_ &m)
{
	return direct_image_reader_provider_class(m, "DirectImageReaderProvider");
}

caching_image_reader_provider_class
declare_caching_image_reader_provider(pybind11::module_ &m)
{
	return caching_image_reader_provider_class(m, "CachingImageReaderProvider");
}

void define_image_reader_provider(pybind11::module_ &m)
{
	m.def(
		"query_descriptor", &em::query_descriptor,
		py::arg("readers"), py::arg("path"),
		py::call_guard<py::gil_scoped_release>()
	);
}

void define_direct_image_reader_provider(
	direct_image_reader_provider_class &c
)
{
	c.def(
		py::init<std::shared_ptr<const em::image_read_format_manager>>(),
		py::arg("formats")
	);
}

void define_caching_image_reader_provider(
	caching_image_reader_provider_class &c
)
{
	c
		.def(
			py::init<
				std::shared_ptr<em::image_reader_provider>,
				std::size_t
			>(),
			py::arg("backing"), py::arg("capacity")
		)
		.def_property_readonly(
			"capacity",
			&em::caching_image_reader_provider::get_capacity
		)
		.def_property_readonly(
			"reader_count",
			&em::caching_image_reader_provider::get_reader_count
		);
}

} // namespace rexlib
