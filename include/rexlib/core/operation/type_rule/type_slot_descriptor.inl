// SPDX-License-Identifier: GPL-3.0-only

#include "type_slot_descriptor.hpp"

namespace rexlib
{

inline
type_slot_descriptor::type_slot_descriptor(
	std::size_t pivot_index,
	const type_transform &transform
) noexcept
	: m_pivot_index(pivot_index)
	, m_transform(transform)
{
}

inline
std::size_t type_slot_descriptor::get_pivot_index() const noexcept
{
	return m_pivot_index;
}

inline
const type_transform& type_slot_descriptor::get_transform() const noexcept
{
	return m_transform;
}

} // namespace rexlib
