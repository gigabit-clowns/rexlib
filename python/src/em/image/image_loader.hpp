// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/executor_image_loader.hpp>
#include <rexlib/em/image/image_loader.hpp>

#include <memory>

namespace rexlib
{

using image_loader_class =
	pybind11::class_<em::image_loader, std::shared_ptr<em::image_loader>>;
using executor_image_loader_class = pybind11::class_<
	em::executor_image_loader,
	em::image_loader,
	std::shared_ptr<em::executor_image_loader>
>;

image_loader_class declare_image_loader(pybind11::module_ &m);
executor_image_loader_class
declare_executor_image_loader(pybind11::module_ &m);
void define_executor_image_loader(executor_image_loader_class &c);

} // namespace rexlib
