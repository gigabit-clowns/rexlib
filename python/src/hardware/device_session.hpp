// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/hardware/device_session.hpp>

#include <memory>

namespace xmipp4
{
namespace hardware
{

using device_session_class = pybind11::class_<device_session, std::shared_ptr<device_session>>;

device_session_class declare_device_session(pybind11::module_ &m);
void define_device_session(device_session_class &c);

} // namespace hardware
} // namespace xmipp4
