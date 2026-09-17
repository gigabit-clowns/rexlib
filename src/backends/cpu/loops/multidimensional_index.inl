// SPDX-License-Identifier: GPL-3.0-only

#include "multidimensional_index.hpp"

namespace rexlib
{
namespace cpu
{

REXLIB_INLINE_CONSTEXPR
multidimensional_index::multidimensional_index(
	std::size_t rank,
	std::size_t inner_axis
) noexcept
	: multidimensional_index(nullptr, rank, inner_axis, 0)
{
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index::multidimensional_index(
	const std::ptrdiff_t *coordinates,
	std::size_t rank,
	std::size_t inner_axis,
	std::size_t inner_offset
) noexcept
	: m_coordinates(coordinates)
	, m_rank(rank)
	, m_inner_axis(inner_axis)
	, m_inner_offset(inner_offset)
{
}

REXLIB_INLINE_CONSTEXPR
std::size_t multidimensional_index::get_rank() const noexcept
{
	return m_rank;
}

REXLIB_INLINE_CONSTEXPR
std::size_t
multidimensional_index::operator[](std::size_t axis) const noexcept
{
	const auto offset = axis == m_inner_axis ? m_inner_offset : std::size_t(0);
	return static_cast<std::size_t>(m_coordinates[axis]) + offset;
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index
multidimensional_index::rebased(
	const std::ptrdiff_t *coordinates
) const noexcept
{
	return multidimensional_index(coordinates, m_rank, m_inner_axis, 0);
}

REXLIB_INLINE_CONSTEXPR
multidimensional_index
multidimensional_index::advanced(std::size_t count) const noexcept
{
	return multidimensional_index(
		m_coordinates,
		m_rank,
		m_inner_axis,
		m_inner_offset + count
	);
}

} // namespace cpu
} // namespace rexlib
