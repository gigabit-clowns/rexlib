// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_service.hpp>

#include <memory>

namespace rexlib
{

using device_service_class = nanobind::class_<device_service>;

device_service_class declare_device_service(nanobind::module_ &m);
void define_device_service(device_service_class &c, nanobind::module_ &m);

} // namespace rexlib
