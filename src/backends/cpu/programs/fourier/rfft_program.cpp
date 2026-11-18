// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/rfft_operation.hpp>

#include <backends/cpu/skeletons/fourier/fourier_program_builder.hpp>
#include <backends/cpu/skeletons/fourier/real_to_complex_fourier_transform.hpp>
#include <backends/cpu/skeletons/rule_type_dispatch.hpp>
#include <backends/cpu/libraries/pocketfft/fft/fourier_transform_support.hpp>
#include <backends/cpu/libraries/pocketfft/fft/pocketfft_fft.hpp>

namespace rexlib
{
namespace cpu
{

// Only the last transformed axis is halved, every other one keeping its
// extent, which is the shape the operation's signature deduced and the one
// pocketfft writes.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER_EX(
	rfft,
	ops::rfft_operation,
	real_to_complex_fourier_transform<pocketfft_fft>,
	rule_type_dispatch<
		ops::rfft_operation::type_rule_type,
		fourier_transform_support
	>
);

} // namespace cpu
} // namespace rexlib
