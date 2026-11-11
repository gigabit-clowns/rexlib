// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/fft_operation.hpp>

#include <backends/cpu/skeletons/fourier/complex_fourier_transform.hpp>
#include <backends/cpu/skeletons/fourier/fourier_program_builder.hpp>
#include <backends/cpu/skeletons/rule_type_dispatch.hpp>
#include <backends/cpu/libraries/pocketfft/fft/fourier_transform_support.hpp>
#include <backends/cpu/libraries/pocketfft/fft/pocketfft_fft.hpp>

namespace rexlib
{
namespace cpu
{

// A real operand takes the same path as a complex one. It is widened into the
// output and transformed there, which is what lets one transform serve both
// and is why the full spectrum is stored: half of it is redundant, but
// storing only that half is what rfft is for.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER_EX(
	fft,
	ops::fft_operation,
	forward_complex_fourier_transform<pocketfft_fft>,
	rule_type_dispatch<
		ops::fft_operation::type_rule_type,
		fourier_transform_support
	>
);

} // namespace cpu
} // namespace rexlib
