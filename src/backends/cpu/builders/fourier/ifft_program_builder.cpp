// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/ifft_operation.hpp>

#include <backends/cpu/builders/complex_fourier_transform.hpp>
#include <backends/cpu/builders/fourier_program_builder.hpp>
#include <backends/cpu/builders/rule_type_dispatch.hpp>
#include <backends/cpu/kernels/fourier_transform_support.hpp>
#include <backends/cpu/kernels/pocketfft_fft.hpp>

namespace rexlib
{
namespace cpu
{

// The same transform as fft, run the other way round and divided by the
// number of samples that reached each value, which is what makes the two undo
// one another.
REXLIB_REGISTER_FOURIER_PROGRAM_BUILDER_EX(
	ifft,
	ops::ifft_operation,
	inverse_complex_fourier_transform<pocketfft_fft>,
	rule_type_dispatch<
		ops::ifft_operation::type_rule_type,
		fourier_transform_support
	>
);

} // namespace cpu
} // namespace rexlib
