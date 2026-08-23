// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/layout/joint_layout.hpp>

#include "joint_layout_implementation.hpp"

#include <sstream>

namespace xmipp4 
{

joint_layout::joint_layout() = default;

joint_layout::joint_layout(
	std::unique_ptr<const joint_layout_implementation> implementation
)
	: m_implementation(std::move(implementation))
{
}

joint_layout::joint_layout(
	joint_layout&& other
) noexcept = default;

joint_layout::~joint_layout() = default;

joint_layout& 
joint_layout::operator=(joint_layout&& other) noexcept = default;

std::size_t joint_layout::get_rank() const noexcept
{
	if (!m_implementation)
	{
		return 0UL;
	}

	return m_implementation->get_rank();
}

std::size_t joint_layout::get_number_of_operands() const noexcept
{
	if (!m_implementation)
	{
		return 0UL;
	}
	
	return m_implementation->get_number_of_operands();
}

span<const std::size_t> joint_layout::get_extents() const
{
	if (!m_implementation)
	{
		return span<const std::size_t>();
	}
	
	return m_implementation->get_extents();
}

span<const std::ptrdiff_t> 
joint_layout::get_strides(std::size_t operand) const
{
	if (!m_implementation)
	{
		const joint_layout_implementation implementation;
		return implementation.get_strides(operand); // throws
	}

	return m_implementation->get_strides(operand);
}

std::ptrdiff_t joint_layout::get_offset(std::size_t operand) const
{
	if (!m_implementation)
	{
		const joint_layout_implementation implementation;
		return implementation.get_offset(operand); // throws
	}

	return m_implementation->get_offset(operand);
}

std::size_t joint_layout::iter(
	joint_cursor &ite, 
	std::size_t first_dim,
	std::size_t last_dim
) const
{
	if (!m_implementation)
	{
		return 0UL;
	}

	return m_implementation->iter(ite, first_dim, last_dim);
}

std::size_t joint_layout::compute_element_count() const noexcept
{
	if (!m_implementation)
	{
		return 0UL;
	}

	return m_implementation->compute_element_count();
}

std::size_t joint_layout::seek(
	joint_cursor &ite,
	std::size_t position,
	std::size_t first_dim,
	std::size_t last_dim
) const
{
	if (!m_implementation)
	{
		return 0UL;
	}

	return m_implementation->seek(ite, position, first_dim, last_dim);
}

std::size_t joint_layout::next(
	joint_cursor &ite,
	std::size_t n,
	std::size_t first_dim,
	std::size_t last_dim
) const noexcept
{
	if (!m_implementation)
	{
		return 0UL;
	}

	return m_implementation->next(ite, n, first_dim, last_dim);
}

const joint_layout_implementation*
joint_layout::get_implementation() const noexcept
{
	return m_implementation.get();
}

} // namespace xmipp4
