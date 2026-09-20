// SPDX-License-Identifier: GPL-3.0-only

#include "image_read_format_manager.hpp"

#include <rexlib/core/service_catalog.hpp>

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
	const em::image_read_format_manager &formats,
	const std::string &path
)
{
	return to_shape(em::query_extents(formats, path));
}

static py::tuple query_core_extents(
	const em::image_read_format_manager &formats,
	const std::string &path
)
{
	return to_shape(em::query_core_extents(formats, path));
}

static std::shared_ptr<em::image_read_format_manager>
get_image_read_format_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<em::image_read_format_manager>();
}

image_read_format_manager_class
declare_image_read_format_manager(pybind11::module_ &m)
{
	return image_read_format_manager_class(m, "ImageReadFormatManager");
}

void define_image_read_format_manager(pybind11::module_ &m)
{
	m.def("get_image_read_format_manager", &get_image_read_format_manager);
	m.def(
		"query_extents", &query_extents,
		py::arg("formats"), py::arg("path")
	);
	m.def(
		"query_core_extents", &query_core_extents,
		py::arg("formats"), py::arg("path")
	);
}

} // namespace rexlib
