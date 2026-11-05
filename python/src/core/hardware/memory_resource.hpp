// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/memory_resource.hpp>

#include <memory>

namespace rexlib
{

using memory_resource_class = nanobind::class_<memory_resource>;

memory_resource_class declare_memory_resource(nanobind::module_ &m);
void define_memory_resource(memory_resource_class &c, nanobind::module_ &m);

} // namespace rexlib
