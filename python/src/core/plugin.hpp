// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/plugin.hpp>

#include <memory>

namespace rexlib
{

using plugin_class = nanobind::class_<plugin>;

plugin_class declare_plugin(nanobind::module_ &m);
void define_plugin(plugin_class &c);

} // namespace rexlib
