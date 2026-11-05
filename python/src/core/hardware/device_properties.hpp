// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_properties.hpp>

#include <memory>

namespace rexlib
{

using device_properties_class = nanobind::class_<device_properties>;

device_properties_class declare_device_properties(nanobind::module_ &m);
void define_device_properties(device_properties_class &c);

} // namespace rexlib
