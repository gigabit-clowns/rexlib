// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_context.hpp>

#include <memory>

namespace rexlib
{

using device_context_class = nanobind::class_<device_context>;

device_context_class declare_device_context(nanobind::module_ &m);
void define_device_context(device_context_class &c);

} // namespace rexlib
