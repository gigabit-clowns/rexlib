// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/image_descriptor.hpp>

namespace rexlib
{

using image_descriptor_class = pybind11::class_<em::image_descriptor>;

image_descriptor_class declare_image_descriptor(pybind11::module_ &m);
void define_image_descriptor(
	image_descriptor_class &c,
	pybind11::module_ &m
);

} // namespace rexlib
