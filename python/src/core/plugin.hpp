// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <rexlib/core/plugin.hpp>

#include <memory>

namespace rexlib
{

using plugin_class = pybind11::class_<plugin>;

plugin_class declare_plugin(pybind11::module_ &m);
void define_plugin(plugin_class &c);

} // namespace rexlib
