// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_index.hpp>

#include <memory>

namespace rexlib
{

using device_index_class = nanobind::class_<device_index>;

device_index_class declare_device_index(nanobind::module_ &m);
void define_device_index(device_index_class &c);

} // namespace rexlib
