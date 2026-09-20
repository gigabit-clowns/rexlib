// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/image_batch_source.hpp>
#include <rexlib/em/image/image_source.hpp>

#include <memory>

namespace rexlib
{

using image_source_class =
	pybind11::class_<em::image_source, std::shared_ptr<em::image_source>>;
using image_batch_source_class = pybind11::class_<
	em::image_batch_source, std::shared_ptr<em::image_batch_source>
>;

image_source_class declare_image_source(pybind11::module_ &m);
image_batch_source_class declare_image_batch_source(pybind11::module_ &m);
void define_image_source(image_source_class &c);
void define_image_batch_source(image_batch_source_class &c);

} // namespace rexlib
