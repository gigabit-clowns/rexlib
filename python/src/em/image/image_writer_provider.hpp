// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/image_writer_provider.hpp>
#include <rexlib/em/image/managed_image_writer_provider.hpp>

#include <memory>

namespace rexlib
{

using image_writer_provider_class = pybind11::class_<
	em::image_writer_provider, std::shared_ptr<em::image_writer_provider>
>;
using managed_image_writer_provider_class = pybind11::class_<
	em::managed_image_writer_provider,
	em::image_writer_provider,
	std::shared_ptr<em::managed_image_writer_provider>
>;

image_writer_provider_class
declare_image_writer_provider(pybind11::module_ &m);
managed_image_writer_provider_class
declare_managed_image_writer_provider(pybind11::module_ &m);
void define_managed_image_writer_provider(
	managed_image_writer_provider_class &c
);

} // namespace rexlib
