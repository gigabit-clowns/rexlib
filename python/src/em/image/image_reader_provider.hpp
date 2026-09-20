// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/caching_image_reader_provider.hpp>
#include <rexlib/em/image/direct_image_reader_provider.hpp>
#include <rexlib/em/image/image_reader_provider.hpp>

#include <memory>

namespace rexlib
{

using image_reader_provider_class = pybind11::class_<
	em::image_reader_provider, std::shared_ptr<em::image_reader_provider>
>;
using direct_image_reader_provider_class = pybind11::class_<
	em::direct_image_reader_provider,
	em::image_reader_provider,
	std::shared_ptr<em::direct_image_reader_provider>
>;
using caching_image_reader_provider_class = pybind11::class_<
	em::caching_image_reader_provider,
	em::image_reader_provider,
	std::shared_ptr<em::caching_image_reader_provider>
>;

image_reader_provider_class
declare_image_reader_provider(pybind11::module_ &m);
direct_image_reader_provider_class
declare_direct_image_reader_provider(pybind11::module_ &m);
caching_image_reader_provider_class
declare_caching_image_reader_provider(pybind11::module_ &m);
void define_image_reader_provider(pybind11::module_ &m);
void define_direct_image_reader_provider(
	direct_image_reader_provider_class &c
);
void define_caching_image_reader_provider(
	caching_image_reader_provider_class &c
);

} // namespace rexlib
