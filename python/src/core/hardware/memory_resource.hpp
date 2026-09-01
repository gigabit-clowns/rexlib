// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/memory_resource.hpp>

#include <memory>

namespace rexlib
{

using memory_resource_class = pybind11::class_<memory_resource>;

memory_resource_class declare_memory_resource(pybind11::module_ &m);
void define_memory_resource(memory_resource_class &c, pybind11::module_ &m);

} // namespace rexlib
