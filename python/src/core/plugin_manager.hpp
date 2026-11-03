// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/plugin_manager.hpp>

namespace rexlib
{

using plugin_manager_class = nanobind::class_<plugin_manager>;

plugin_manager_class declare_plugin_manager(nanobind::module_ &m);
void define_plugin_manager(plugin_manager_class &c, nanobind::module_ &m);

} // namespace rexlib
