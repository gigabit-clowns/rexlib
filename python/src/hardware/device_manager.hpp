// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device_manager.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using device_manager_class = pybind11::class_<device_manager, std::shared_ptr<device_manager>>;

device_manager_class declare_device_manager(pybind11::module_ &m);
void define_device_manager(device_manager_class &c, pybind11::module_ &m);

} // namespace hardware
} // namespace xmipp4
