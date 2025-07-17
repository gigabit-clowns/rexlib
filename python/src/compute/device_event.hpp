// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

namespace xmipp4
{
namespace compute
{

void bind_device_event(pybind11::module_ &m);

} // namespace compute
} // namespace xmipp4
