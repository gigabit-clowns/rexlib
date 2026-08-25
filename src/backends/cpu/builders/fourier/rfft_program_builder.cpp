// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/rfft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace rexlib
{
namespace cpu
{

// Only the last transformed axis is halved, every other one keeping its
// extent, which is the shape the operation's own policy deduced and the one
// pocketfft writes.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER(
	rfft,
	ops::rfft_operation,
	real_to_complex_fourier_transform
);

} // namespace cpu
} // namespace rexlib
