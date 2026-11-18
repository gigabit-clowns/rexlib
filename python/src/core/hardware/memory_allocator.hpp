// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/memory_allocator.hpp>

#include <memory>

namespace rexlib
{

using memory_allocator_class = nanobind::class_<memory_allocator>;

memory_allocator_class declare_memory_allocator(nanobind::module_ &m);
void define_memory_allocator(memory_allocator_class &c);

} // namespace rexlib
