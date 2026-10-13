// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_backend.hpp>

#include <memory>

namespace rexlib
{

using device_backend_class = nanobind::class_<device_backend>;

device_backend_class declare_device_backend(nanobind::module_ &m);
void define_device_backend(device_backend_class &c);

} // namespace rexlib
