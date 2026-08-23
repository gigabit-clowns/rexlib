// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/fourier/ifft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace xmipp4
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
} // namespace xmipp4
