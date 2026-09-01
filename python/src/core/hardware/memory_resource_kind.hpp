// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/memory_resource_kind.hpp>

#include <memory>

namespace rexlib
{

using memory_resource_kind_class = pybind11::enum_<memory_resource_kind>;

memory_resource_kind_class declare_memory_resource_kind(pybind11::module_ &m);
void define_memory_resource_kind(memory_resource_kind_class &c);

} // namespace rexlib
