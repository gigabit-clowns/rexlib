// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/image_location.hpp>

namespace rexlib
{

using image_location_class = pybind11::class_<em::image_location>;

image_location_class declare_image_location(pybind11::module_ &m);
void define_image_location(image_location_class &c, pybind11::module_ &m);

} // namespace rexlib
