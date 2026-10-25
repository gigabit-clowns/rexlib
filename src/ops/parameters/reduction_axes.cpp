// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/parameters/reduction_axes.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

reduction_axes::reduction_axes(
	axis_list axes,
	bool keep_dimensions
) noexcept
	: m_axes(std::move(axes))
	, m_keep_dimensions(keep_dimensions)
{
}

span<const std::size_t> reduction_axes::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

bool reduction_axes::get_keep_dimensions() const noexcept
{
	return m_keep_dimensions;
}

} // namespace ops
} // namespace rexlib
