// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

namespace xmipp4
{
namespace ndarray
{

void bind_array_descriptor(pybind11::module_ &m);

} // namespace ndarray
} // namespace xmipp4
