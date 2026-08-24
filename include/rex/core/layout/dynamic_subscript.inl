// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "dynamic_subscript.hpp"

#include <sstream>

namespace rex 
{

REXLIB_INLINE_CONSTEXPR
dynamic_subscript::dynamic_subscript(ellipsis_tag) noexcept
	: m_data{}
	, m_type(subscript_type::ellipsis)
{
}

REXLIB_INLINE_CONSTEXPR
dynamic_subscript::dynamic_subscript(new_axis_tag) noexcept
	: m_data{}
	, m_type(subscript_type::new_axis)
{
}

REXLIB_INLINE_CONSTEXPR
dynamic_subscript::dynamic_subscript(std::ptrdiff_t index) noexcept
	: m_data{index}
	, m_type(subscript_type::index)
{
}

REXLIB_INLINE_CONSTEXPR
dynamic_subscript::dynamic_subscript(const slice &slice) noexcept
	: m_data{
		slice.get_start(), 
		static_cast<std::ptrdiff_t>(slice.get_count()), 
		slice.get_step()
	}
	, m_type(subscript_type::slice)
{
}

REXLIB_INLINE_CONSTEXPR
dynamic_subscript::subscript_type 
dynamic_subscript::get_subscript_type() const noexcept
{
	return m_type;
}

inline
std::ptrdiff_t dynamic_subscript::get_index() const
{
	if (m_type != subscript_type::index)
	{
		std::ostringstream oss;
		oss << "Cannot call get_index on a " << *this;
		throw bad_dynamic_subscript_access(oss.str());
	}

	return m_data[0];
}

inline
slice dynamic_subscript::get_slice() const
{
	if (m_type != subscript_type::slice)
	{
		std::ostringstream oss;
		oss << "Cannot call get_slice on a " << *this;
		throw bad_dynamic_subscript_access(oss.str());
	}

	return make_slice(m_data[0], m_data[1], m_data[2]);
}

template <typename F>
REXLIB_INLINE_CONSTEXPR
auto visit(F&& func, const dynamic_subscript &subscript)
{
	switch (subscript.get_subscript_type())
	{
	case dynamic_subscript::subscript_type::ellipsis:
		return std::forward<F>(func)(ellipsis());
	case dynamic_subscript::subscript_type::new_axis:
		return std::forward<F>(func)(new_axis());
	case dynamic_subscript::subscript_type::index:
		return std::forward<F>(func)(subscript.get_index());
	case dynamic_subscript::subscript_type::slice:
		return std::forward<F>(func)(subscript.get_slice());
	}
}

} // namespace rex
