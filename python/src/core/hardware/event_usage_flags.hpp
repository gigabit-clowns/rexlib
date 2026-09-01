// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/hardware/event_usage_flags.hpp>

namespace rexlib
{

using event_usage_flag_bits_class = pybind11::enum_<event_usage_flag_bits>;
using event_usage_flags_class = pybind11::class_<event_usage_flags>;

event_usage_flag_bits_class declare_event_usage_flag_bits(pybind11::module_ &m);
void define_event_usage_flag_bits(event_usage_flag_bits_class &c);

event_usage_flags_class declare_event_usage_flags(pybind11::module_ &m);
void define_event_usage_flags(event_usage_flags_class &c);

} // namespace rexlib
