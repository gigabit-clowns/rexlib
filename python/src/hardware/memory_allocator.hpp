// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/memory_allocator.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using memory_allocator_class = pybind11::class_<memory_allocator, std::shared_ptr<memory_allocator>>;

memory_allocator_class declare_memory_allocator(pybind11::module_ &m);
void define_memory_allocator(memory_allocator_class &c);

} // namespace hardware
} // namespace xmipp4
