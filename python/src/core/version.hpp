// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/version.hpp>

namespace xmipp4
{

using version_class = pybind11::class_<version>;

version_class declare_version(pybind11::module_ &m);
void define_version(version_class &c);

} // namespace xmipp4
