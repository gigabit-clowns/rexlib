// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <bitset>

namespace rexlib
{

class numerical_type_promotion_lattice
{
public:
	REXLIB_CONSTEXPR_CPP23
	numerical_type_promotion_lattice() noexcept;

	REXLIB_CONSTEXPR_CPP23
	numerical_type_promotion_lattice& 
	add_edge(numerical_type from, numerical_type to) noexcept;

	REXLIB_CONSTEXPR_CPP23
	numerical_type 
	get_supremum(numerical_type a, numerical_type b) const noexcept;

private:
	using reach_set = 
		std::bitset<static_cast<std::size_t>(numerical_type::count)>;

	std::array<
		reach_set, 
		static_cast<std::size_t>(numerical_type::count)
	> m_reach;

	REXLIB_CONSTEXPR_CPP23
	bool is_supremum(const reach_set &common, std::size_t i) const noexcept;
};

} // namespace rexlib

#include "numerical_type_promotion_lattice.inl"
