// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/ndarray/array_descriptor.hpp>

namespace rexlib
{

using array_descriptor_class = nanobind::class_<array_descriptor>;

array_descriptor_class declare_array_descriptor(nanobind::module_ &m);
void define_array_descriptor(array_descriptor_class &c, nanobind::module_ &m);

} // namespace rexlib
