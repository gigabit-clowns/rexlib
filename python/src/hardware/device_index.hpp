// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device_index.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using device_index_class = pybind11::class_<device_index>;

device_index_class declare_device_index(pybind11::module_ &m);
void define_device_index(device_index_class &c);

} // namespace hardware
} // namespace xmipp4
