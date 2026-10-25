// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/parameters/restored_signal_axes.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

restored_signal_axes::restored_signal_axes(
	axis_list axes,
	std::size_t extent
) noexcept
	: m_axes(std::move(axes))
	, m_extent(extent)
{
}

span<const std::size_t> restored_signal_axes::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

std::size_t restored_signal_axes::get_extent() const noexcept
{
	return m_extent;
}

} // namespace ops
} // namespace rexlib
