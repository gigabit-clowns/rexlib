// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/fft_operation.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

fft_operation::fft_operation(
	axis_list axes,
	fourier_normalization normalization
)
	: parametric_operation(std::move(axes))
	, m_normalization(normalization)
{
}

fourier_normalization fft_operation::get_normalization() const noexcept
{
	return m_normalization;
}

} // namespace ops
} // namespace rexlib
