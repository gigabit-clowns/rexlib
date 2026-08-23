// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/ifft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace rex
{
namespace cpu
{

// The same transform as fft, run the other way round and divided by the
// number of samples that reached each value, which is what makes the two undo
// one another.
REX_REGISTER_FOURIER_PROGRAM_BUILDER(
	ifft,
	ops::ifft_operation,
	inverse_complex_fourier_transform
);

} // namespace cpu
} // namespace rex
