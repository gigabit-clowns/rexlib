// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_manager.hpp>

#include <memory>

namespace rexlib
{

using device_manager_class = nanobind::class_<device_manager>;

device_manager_class declare_device_manager(nanobind::module_ &m);
void define_device_manager(device_manager_class &c, nanobind::module_ &m);

} // namespace rexlib
