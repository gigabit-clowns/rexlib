// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/parameters/axis_selection.hpp>

#include <utility>

namespace rexlib
{
namespace ops
{

axis_selection::axis_selection(axis_list axes) noexcept
	: m_axes(std::move(axes))
{
}

span<const std::size_t> axis_selection::get_axes() const noexcept
{
	return make_span(m_axes.data(), m_axes.size());
}

} // namespace ops
} // namespace rexlib
