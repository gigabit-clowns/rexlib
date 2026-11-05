// SPDX-License-Identifier: GPL-3.0-only

#include "fourier_normalization.hpp"

namespace rexlib
{
namespace ops
{

namespace nb = nanobind;

fourier_normalization_class declare_fourier_normalization(nb::module_ &m)
{
	return fourier_normalization_class(m, "FourierNormalization");
}

void define_fourier_normalization(fourier_normalization_class &c)
{
	c
		.value("backward", fourier_normalization::backward)
		.value("ortho", fourier_normalization::ortho)
		.value("forward", fourier_normalization::forward);
}

} // namespace ops
} // namespace rexlib
