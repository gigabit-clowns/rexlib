// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

namespace xmipp4
{
namespace dispatch
{

void bind_dispatcher(pybind11::module_ &m);

} // namespace dispatch
} // namespace xmipp4
