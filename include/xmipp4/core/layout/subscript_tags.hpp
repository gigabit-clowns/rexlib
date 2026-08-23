// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "slice.hpp"
#include "../platform/constexpr.hpp"

#include <ostream>
#include <type_traits>

namespace xmipp4 
{

/**
 * @brief Tag to express the consumption of an arbitrary amount 
 * of axes until no slack is left on the right-hand dimensions.
 * 
 * NumPy equivalent of Ellipsis or "...".
 * 
 */
struct ellipsis_tag
{
	friend REX_CONSTEXPR
	bool operator==(ellipsis_tag, ellipsis_tag) noexcept
	{
		return true;
	}

	friend REX_CONSTEXPR
	bool operator!=(ellipsis_tag, ellipsis_tag) noexcept
	{
		return false;
	}

	template <typename T>
	friend std::basic_ostream<T>&
	operator<<(std::basic_ostream<T> &os, ellipsis_tag)
	{
		return os << "ellipsis_tag";
	}
};

/**
 * @brief Construct an ellipsis_tag.
 * 
 * @return ellipsis_tag Newly constructed tag.
 */
REX_CONSTEXPR
ellipsis_tag ellipsis() noexcept;

/**
 * @brief Tag to express the creation a new axis of size one.
 * 
 * NumPy equivalent of numpy.newaxis() or "None".
 * 
 */
struct new_axis_tag
{
	friend REX_CONSTEXPR
	bool operator==(new_axis_tag, new_axis_tag) noexcept
	{
		return true;
	}

	friend REX_CONSTEXPR
	bool operator!=(new_axis_tag, new_axis_tag) noexcept
	{
		return false;
	}

	template <typename T>
	friend std::basic_ostream<T>&
	operator<<(std::basic_ostream<T> &os, new_axis_tag)
	{
		return os << "new_axis_tag";
	}
};

/**
 * @brief Construct a new_axis_tag.
 * 
 * @return new_axis_tag Newly constructed tag.
 */
REX_CONSTEXPR
new_axis_tag new_axis() noexcept;

} // namespace xmipp4

#include "subscript_tags.inl"
