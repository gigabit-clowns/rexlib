// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "functional/main.hpp"
#include "ops/main.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_em(nb::module_ &m)
{
	auto ops_module = m.def_submodule("ops");
	bind_ops(ops_module);

	auto functional_module = m.def_submodule("functional");
	bind_functional(functional_module);
}

} // namespace em
} // namespace rexlib
