// SPDX-License-Identifier: GPL-3.0-only

#include "joint_layout_operand.hpp"

namespace rexlib 
{

inline
joint_layout_operand::joint_layout_operand(
	const stride_vector_type &strides,
	std::ptrdiff_t offset
)
	: m_strides(strides)
	, m_offset(offset)
{
}

inline
span<const std::ptrdiff_t> 
joint_layout_operand::get_strides() const noexcept
{
	return span<const std::ptrdiff_t>(m_strides.data(), m_strides.size());
}

inline
std::ptrdiff_t joint_layout_operand::get_offset() const noexcept
{
	return m_offset;
}

inline
int joint_layout_operand::compare_strides(
	std::size_t i, 
	std::size_t j
) const noexcept
{
	const auto stride_i = m_strides[i];
	const auto stride_j = m_strides[j];

	if (stride_i == 0 || stride_j == 0)
	{
		return 0;
	}
	
	return static_cast<int>(stride_i - stride_j);
}

inline
void joint_layout_operand::swap_axes(
	std::size_t i, 
	std::size_t j
) noexcept
{
	std::swap(m_strides[i], m_strides[j]);
}

inline
void joint_layout_operand::trim_axes(std::size_t n)
{
	m_strides.resize(n);
}

} // namespace rexlib
