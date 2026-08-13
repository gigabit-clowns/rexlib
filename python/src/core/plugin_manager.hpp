// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/plugin_manager.hpp>

namespace xmipp4
{

using plugin_manager_class = pybind11::class_<plugin_manager>;

plugin_manager_class declare_plugin_manager(pybind11::module_ &m);
void define_plugin_manager(plugin_manager_class &c, pybind11::module_ &m);

} // namespace xmipp4
