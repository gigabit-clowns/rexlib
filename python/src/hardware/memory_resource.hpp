// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/memory_resource.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using memory_resource_class = pybind11::class_<memory_resource>;

memory_resource_class declare_memory_resource(pybind11::module_ &m);
void define_memory_resource(memory_resource_class &c, pybind11::module_ &m);

} // namespace hardware
} // namespace xmipp4
