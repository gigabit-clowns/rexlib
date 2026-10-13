// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_type.hpp>

namespace rexlib
{

using device_type_class = nanobind::enum_<device_type>;

device_type_class declare_device_type(nanobind::module_ &m);
void define_device_type(device_type_class &c);

} // namespace rexlib
