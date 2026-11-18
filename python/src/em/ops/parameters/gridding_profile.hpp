// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/em/ops/parameters/gridding_profile.hpp>

namespace rexlib
{
namespace em
{

using gridding_profile_class = nanobind::enum_<gridding_profile>;

gridding_profile_class declare_gridding_profile(nanobind::module_ &m);
void define_gridding_profile(gridding_profile_class &c);

} // namespace em
} // namespace rexlib
