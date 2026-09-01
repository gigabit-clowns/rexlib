// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/device_session.hpp>

#include <memory>

namespace rexlib
{

using device_session_class = pybind11::class_<device_session, std::shared_ptr<device_session>>;

device_session_class declare_device_session(pybind11::module_ &m);
void define_device_session(device_session_class &c);

} // namespace rexlib
