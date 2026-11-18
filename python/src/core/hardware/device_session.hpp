// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/hardware/device_session.hpp>

#include <memory>

namespace rexlib
{

using device_session_class = nanobind::class_<device_session>;

device_session_class declare_device_session(nanobind::module_ &m);
void define_device_session(device_session_class &c);

} // namespace rexlib
