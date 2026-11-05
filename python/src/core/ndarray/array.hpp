// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/ndarray/array.hpp>

namespace rexlib
{

using array_class = nanobind::class_<array>;

array_class declare_array(nanobind::module_ &m);
void define_array(array_class &c);

} // namespace rexlib
