// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/irfft_operation.hpp>

#include <backends/cpu/skeletons/fourier/complex_to_real_fourier_transform.hpp>
#include <backends/cpu/skeletons/fourier/fourier_program_builder.hpp>
#include <backends/cpu/skeletons/rule_type_dispatch.hpp>
#include <backends/cpu/libraries/pocketfft/fft/fourier_transform_support.hpp>
#include <backends/cpu/libraries/pocketfft/fft/pocketfft_fft.hpp>

namespace rexlib
{
namespace cpu
{

// The restored extent comes from the output, which the operation signature
// sized from the extent the operation carries.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER_EX(
	irfft,
	ops::irfft_operation,
	complex_to_real_fourier_transform<pocketfft_fft>,
	rule_type_dispatch<
		ops::irfft_operation::type_rule_type,
		fourier_transform_support
	>
);

} // namespace cpu
} // namespace rexlib
