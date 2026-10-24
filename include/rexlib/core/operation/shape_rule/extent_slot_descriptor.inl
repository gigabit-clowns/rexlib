// SPDX-License-Identifier: GPL-3.0-only

#include "extent_slot_descriptor.hpp"

namespace rexlib
{

inline
extent_slot_descriptor::extent_slot_descriptor(
	std::size_t pivot_index,
	const extent_transform &transform
) noexcept
	: m_pivot_index(pivot_index)
	, m_transform(transform)
{
}

inline
std::size_t extent_slot_descriptor::get_pivot_index() const noexcept
{
	return m_pivot_index;
}

inline
const extent_transform&
extent_slot_descriptor::get_transform() const noexcept
{
	return m_transform;
}

} // namespace rexlib
