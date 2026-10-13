// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/memory_resource_kind.hpp>

#include <memory>

namespace rexlib
{

using memory_resource_kind_class = nanobind::enum_<memory_resource_kind>;

memory_resource_kind_class declare_memory_resource_kind(nanobind::module_ &m);
void define_memory_resource_kind(memory_resource_kind_class &c);

} // namespace rexlib
