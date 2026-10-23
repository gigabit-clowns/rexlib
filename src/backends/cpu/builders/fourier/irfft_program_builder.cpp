// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/irfft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace rexlib
{
namespace cpu
{

// The restored extent comes from the output, which the operand contract
// sized from the extent the operation carries.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER(
	irfft,
	ops::irfft_operation,
	complex_to_real_fourier_transform
);

} // namespace cpu
} // namespace rexlib
