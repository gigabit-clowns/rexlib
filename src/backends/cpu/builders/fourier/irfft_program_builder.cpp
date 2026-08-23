// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/irfft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace rex
{
namespace cpu
{

// The restored extent is not a decision this builder makes: the parity the
// operation carries is what the shape policy deduced the output from, so the
// output is already the right size by the time the plan reads it.
REX_REGISTER_FOURIER_PROGRAM_BUILDER(
	irfft,
	ops::irfft_operation,
	complex_to_real_fourier_transform
);

} // namespace cpu
} // namespace rex
