// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/event.hpp>

#include <memory>

namespace rexlib
{

using event_class = pybind11::class_<event, std::shared_ptr<event>>;

event_class declare_event(pybind11::module_ &m);
void define_event(event_class &c);

} // namespace rexlib
