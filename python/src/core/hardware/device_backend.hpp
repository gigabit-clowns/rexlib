// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/device_backend.hpp>

#include <memory>

namespace rexlib
{

using device_backend_class = pybind11::class_<device_backend>;

device_backend_class declare_device_backend(pybind11::module_ &m);
void define_device_backend(device_backend_class &c);

} // namespace rexlib
