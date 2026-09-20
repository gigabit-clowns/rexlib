// SPDX-License-Identifier: GPL-3.0-only

#include "image_reader_provider.hpp"

#include <rexlib/em/image/image_read_format_manager.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace rexlib
{

namespace py = pybind11;

static py::tuple to_shape(const std::vector<std::size_t> &extents)
{
	py::tuple shape(extents.size());
	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		shape[i] = extents[i];
	}
	return shape;
}

static py::tuple query_extents(
	em::image_reader_provider &readers,
	const std::string &path
)
{
	return to_shape(em::query_extents(readers, path));
}

static py::tuple query_core_extents(
	em::image_reader_provider &readers,
	const std::string &path
)
{
	return to_shape(em::query_core_extents(readers, path));
}

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
		"query_extents", &query_extents,
		py::arg("readers"), py::arg("path")
	);
	m.def(
		"query_core_extents", &query_core_extents,
		py::arg("readers"), py::arg("path")
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
	c.def(
		py::init<std::shared_ptr<em::image_reader_provider>, std::size_t>(),
		py::arg("backing"), py::arg("capacity")
	);
}

} // namespace rexlib
