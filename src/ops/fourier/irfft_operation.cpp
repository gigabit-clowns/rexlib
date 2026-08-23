// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/irfft_operation.hpp>

#include <utility>

namespace rex
{
namespace ops
{

irfft_operation::irfft_operation(
	axis_list axes,
	signal_parity parity,
	fourier_normalization normalization
)
	: parametric_operation(std::move(axes), parity)
	, m_normalization(normalization)
{
}

fourier_normalization irfft_operation::get_normalization() const noexcept
{
	return m_normalization;
}

} // namespace ops
} // namespace rex
