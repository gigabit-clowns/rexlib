// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device_type.hpp>

namespace xmipp4
{
namespace hardware
{

using device_type_class = pybind11::enum_<device_type>;

device_type_class declare_device_type(pybind11::module_ &m);
void define_device_type(device_type_class &c);

} // namespace hardware
} // namespace xmipp4
