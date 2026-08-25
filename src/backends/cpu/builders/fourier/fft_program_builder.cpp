// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/fft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace rexlib
{
namespace cpu
{

// A real operand takes the same path as a complex one. It is widened into the
// output and transformed there, which is what lets one transform serve both
// and is why the full spectrum is stored: half of it is redundant, but
// storing only that half is what rfft is for.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER(
	fft,
	ops::fft_operation,
	forward_complex_fourier_transform
);

} // namespace cpu
} // namespace rexlib
