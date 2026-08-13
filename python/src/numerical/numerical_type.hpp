// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

#include <xmipp4/core/numerical/numerical_type.hpp>

#include <memory>

namespace xmipp4
{
namespace numerical
{

using numerical_type_class = pybind11::enum_<numerical_type>;

numerical_type_class declare_numerical_type(pybind11::module_ &m);
void define_numerical_type(numerical_type_class &c);

} // namespace numerical
} // namespace xmipp4
