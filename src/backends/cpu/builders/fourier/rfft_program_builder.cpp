// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/fourier/rfft_operation.hpp>

#include <backends/cpu/builders/fourier_program_builder.hpp>

namespace xmipp4
{
namespace cpu
{

// Only the last transformed axis is halved, every other one keeping its
// extent, which is the shape the operation's own policy deduced and the one
// pocketfft writes.
REX_REGISTER_FOURIER_PROGRAM_BUILDER(
	rfft,
	ops::rfft_operation,
	real_to_complex_fourier_transform
);

} // namespace cpu
} // namespace xmipp4
