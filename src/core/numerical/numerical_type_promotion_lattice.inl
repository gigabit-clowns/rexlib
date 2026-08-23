// SPDX-License-Identifier: GPL-3.0-only

#include "numerical_type_promotion_lattice.hpp"

namespace xmipp4
{

REX_INLINE_CONSTEXPR_CPP23
numerical_type_promotion_lattice::numerical_type_promotion_lattice() noexcept
{
	for (std::size_t i = 0; i < m_reach.size(); ++i)
	{
		m_reach[i].set(i);
	}
}

REX_INLINE_CONSTEXPR_CPP23
numerical_type_promotion_lattice& 
numerical_type_promotion_lattice::add_edge(
	numerical_type from, 
	numerical_type to
) noexcept
{
	const auto u = static_cast<std::size_t>(to);
	const auto v = static_cast<std::size_t>(from);

	if (m_reach[u].test(v))
	{
		return *this; // Already set
	}

	for (std::size_t i = 0; i < m_reach.size(); ++i) 
	{
		if (m_reach[i].test(v)) 
		{
			m_reach[i] |= m_reach[u];
		}
	}

	return *this;
}

REX_INLINE_CONSTEXPR_CPP23
numerical_type numerical_type_promotion_lattice::get_supremum(
	numerical_type a, 
	numerical_type b
) const noexcept
{
	const auto common = 
		m_reach[static_cast<std::size_t>(a)] & 
		m_reach[static_cast<std::size_t>(b)];

	for (std::size_t i = 0; i < common.size(); ++i) 
	{
		if (is_supremum(common, i)) 
		{
			return static_cast<numerical_type>(i);
		}
	}

	return numerical_type::unknown;
}

REX_INLINE_CONSTEXPR_CPP23
bool numerical_type_promotion_lattice::is_supremum(
	const reach_set &common, 
	std::size_t i
) const noexcept
{
	if (!common.test(i))
	{
		return false;
	}

	for (std::size_t j = 0; j < common.size(); ++j) 
	{
		if (common[j] && !m_reach[i].test(j)) 
		{
			return false;
		}
	}

	return true;
}

} // namespace xmipp4
