// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device_properties.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using device_properties_class = pybind11::class_<device_properties>;

device_properties_class declare_device_properties(pybind11::module_ &m);
void define_device_properties(device_properties_class &c);

} // namespace hardware
} // namespace xmipp4
