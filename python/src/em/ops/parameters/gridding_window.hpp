// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/em/ops/parameters/gridding_window.hpp>

namespace rexlib
{
namespace em
{

using gridding_window_class = nanobind::class_<gridding_window>;

gridding_window_class declare_gridding_window(nanobind::module_ &m);
void define_gridding_window(gridding_window_class &c);

} // namespace em
} // namespace rexlib
