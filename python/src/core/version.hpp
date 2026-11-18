// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/version.hpp>

namespace rexlib
{

using version_class = nanobind::class_<version>;

version_class declare_version(nanobind::module_ &m);
void define_version(version_class &c, nanobind::module_ &m);

} // namespace rexlib
