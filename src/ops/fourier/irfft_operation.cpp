// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/irfft_operation.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

irfft_operation::irfft_operation(
	axis_list axes,
	std::size_t extent,
	fourier_normalization normalization
)
	: parametric_operation(std::move(axes), extent)
	, m_normalization(normalization)
{
}

fourier_normalization irfft_operation::get_normalization() const noexcept
{
	return m_normalization;
}

} // namespace ops
} // namespace rexlib
