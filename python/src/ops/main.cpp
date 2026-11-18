// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "fourier/main.hpp"

namespace rexlib
{
namespace ops
{

namespace nb = nanobind;

void bind_ops(nb::module_ &m)
{
	auto fourier_module = m.def_submodule("fourier");
	bind_fourier(fourier_module);
}

} // namespace ops
} // namespace rexlib
