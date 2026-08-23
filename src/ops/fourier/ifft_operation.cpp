// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/fourier/ifft_operation.hpp>

#include <utility>

namespace rex
{
namespace ops
{

ifft_operation::ifft_operation(
	axis_list axes,
	fourier_normalization normalization
)
	: parametric_operation(std::move(axes))
	, m_normalization(normalization)
{
}

fourier_normalization ifft_operation::get_normalization() const noexcept
{
	return m_normalization;
}

} // namespace ops
} // namespace rex
