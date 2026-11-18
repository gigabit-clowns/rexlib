// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "fourier_normalization.hpp"

namespace rexlib
{
namespace ops
{

namespace nb = nanobind;

void bind_fourier(nb::module_ &m)
{
	auto fourier_normalization = declare_fourier_normalization(m);

	define_fourier_normalization(fourier_normalization);
}

} // namespace ops
} // namespace rexlib
