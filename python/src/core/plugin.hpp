// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/plugin.hpp>

#include <memory>

namespace xmipp4
{

using plugin_class = pybind11::class_<plugin>;

plugin_class declare_plugin(pybind11::module_ &m);
void define_plugin(plugin_class &c);

} // namespace xmipp4
