// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <nanobind/nanobind.h>

#include <rexlib/core/numerical/numerical_type.hpp>

#include <memory>

namespace rexlib
{

using numerical_type_class = nanobind::enum_<numerical_type>;

numerical_type_class declare_numerical_type(nanobind::module_ &m);
void define_numerical_type(numerical_type_class &c);

} // namespace rexlib
