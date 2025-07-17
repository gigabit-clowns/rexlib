// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <pybind11/pybind11.h>

namespace xmipp4
{
namespace communication
{

void bind_communication(pybind11::module_ &m);

} // namespace communication
} // namespace xmipp4
