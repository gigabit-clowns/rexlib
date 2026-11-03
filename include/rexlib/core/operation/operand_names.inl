// SPDX-License-Identifier: GPL-3.0-only

#include "operand_names.hpp"

namespace rexlib
{

template <std::size_t N>
REXLIB_INLINE_CONSTEXPR
operand_names<N>::operand_names(std::array<const char*, N> names) noexcept
	: m_names(names)
{
}

template <std::size_t N>
inline
span<const char* const> operand_names<N>::get() const noexcept
{
	return make_span(m_names.data(), N);
}

template <typename... Names>
REXLIB_INLINE_CONSTEXPR
operand_names<sizeof...(Names)> make_operand_names(Names... names) noexcept
{
	return operand_names<sizeof...(Names)>(
		std::array<const char*, sizeof...(Names)>{ { names... } }
	);
}

} // namespace rexlib
