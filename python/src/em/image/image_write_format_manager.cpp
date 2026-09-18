// SPDX-License-Identifier: GPL-3.0-only

#include "image_write_format_manager.hpp"

#include <rexlib/core/service_catalog.hpp>

namespace rexlib
{

static std::shared_ptr<em::image_write_format_manager>
get_image_write_format_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<em::image_write_format_manager>();
}

image_write_format_manager_class
declare_image_write_format_manager(pybind11::module_ &m)
{
	return image_write_format_manager_class(m, "ImageWriteFormatManager");
}

void define_image_write_format_manager(pybind11::module_ &m)
{
	m.def("get_image_write_format_manager", &get_image_write_format_manager);
}

} // namespace rexlib
