// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/device_context.hpp>

#include <memory>

namespace rexlib
{

using device_context_class = pybind11::class_<device_context>;

device_context_class declare_device_context(pybind11::module_ &m);
void define_device_context(device_context_class &c);

} // namespace rexlib
