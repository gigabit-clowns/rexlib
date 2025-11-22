// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

namespace xmipp4
{
namespace hardware
{

void bind_hardware(pybind11::module_ &m);

} // namespace hardware
} // namespace xmipp4
