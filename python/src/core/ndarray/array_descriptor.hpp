// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <memory>

namespace rexlib
{

using array_descriptor_class = pybind11::class_<array_descriptor>;

array_descriptor_class declare_array_descriptor(pybind11::module_ &m);
void define_array_descriptor(array_descriptor_class &c, pybind11::module_ &m);

} // namespace rexlib
