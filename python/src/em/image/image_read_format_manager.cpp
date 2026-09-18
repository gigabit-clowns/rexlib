// SPDX-License-Identifier: GPL-3.0-only

#include "image_read_format_manager.hpp"

#include <rexlib/core/service_catalog.hpp>

namespace rexlib
{

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
}

} // namespace rexlib
