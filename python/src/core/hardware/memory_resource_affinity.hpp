// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/memory_resource_affinity.hpp>

#include <memory>

namespace rexlib
{

using memory_resource_affinity_class = pybind11::enum_<memory_resource_affinity>;

memory_resource_affinity_class declare_memory_resource_affinity(pybind11::module_ &m);
void define_memory_resource_affinity(memory_resource_affinity_class &c);

} // namespace rexlib
