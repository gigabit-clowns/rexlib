// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/em/image/executor_image_saver.hpp>
#include <rexlib/em/image/image_saver.hpp>

#include <memory>

namespace rexlib
{

using image_saver_class =
	pybind11::class_<em::image_saver, std::shared_ptr<em::image_saver>>;
using executor_image_saver_class = pybind11::class_<
	em::executor_image_saver,
	em::image_saver,
	std::shared_ptr<em::executor_image_saver>
>;

image_saver_class declare_image_saver(pybind11::module_ &m);
executor_image_saver_class declare_executor_image_saver(pybind11::module_ &m);
void define_executor_image_saver(executor_image_saver_class &c);

} // namespace rexlib
