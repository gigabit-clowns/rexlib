// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/layout/joint_cursor.hpp>

namespace rex 
{

joint_cursor::joint_cursor() noexcept = default;
joint_cursor::joint_cursor(
	std::size_t ndim,
	std::vector<std::ptrdiff_t> offsets
)
	: m_indices(ndim, 0UL)
	, m_offsets(std::move(offsets))
{
}

joint_cursor::joint_cursor(
	const joint_cursor &other
) = default;
joint_cursor::joint_cursor(
	joint_cursor &&other
) noexcept = default;
joint_cursor::~joint_cursor() = default;

joint_cursor& 
joint_cursor::operator=(const joint_cursor &other) = default;
joint_cursor& 
joint_cursor::operator=(joint_cursor &&other) noexcept = default;

span<const std::size_t> joint_cursor::get_indices() const noexcept
{
	return make_span(m_indices);
}

span<std::size_t> joint_cursor::get_indices() noexcept
{
	return make_span(m_indices);
}

span<const std::ptrdiff_t> joint_cursor::get_offsets() const noexcept
{
	return make_span(m_offsets);
}

span<std::ptrdiff_t> joint_cursor::get_offsets() noexcept
{
	return make_span(m_offsets);
}

} // namespace rex
