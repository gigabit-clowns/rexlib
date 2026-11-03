// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "parameters/main.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_ops(nb::module_ &m)
{
	auto parameters_module = m.def_submodule("parameters");
	bind_parameters(parameters_module);
}

} // namespace em
} // namespace rexlib
