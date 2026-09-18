// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/image_read_format_manager.hpp>

#include <memory>

namespace rexlib
{

using image_read_format_manager_class = pybind11::class_<
	em::image_read_format_manager,
	std::shared_ptr<em::image_read_format_manager>
>;

image_read_format_manager_class
declare_image_read_format_manager(pybind11::module_ &m);
void define_image_read_format_manager(pybind11::module_ &m);

} // namespace rexlib
