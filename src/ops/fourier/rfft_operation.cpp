// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/fourier/rfft_operation.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

rfft_operation::rfft_operation(
	axis_list axes,
	fourier_normalization normalization
)
	: parametric_operation(std::move(axes))
	, m_normalization(normalization)
{
}

fourier_normalization rfft_operation::get_normalization() const noexcept
{
	return m_normalization;
}

} // namespace ops
} // namespace rexlib
