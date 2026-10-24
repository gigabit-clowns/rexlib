// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{

/**
 * @brief Accessor yielding a fixed extent, whatever the parameters.
 *
 * @tparam N The extent.
 *
 * @see extent_fixed
 */
template <std::size_t N>
struct fixed_extent
{
	/**
	 * @brief Get the extent.
	 *
	 * @tparam Parameters Parameters of the operation instance. Unused.
	 * @return std::size_t N.
	 */
	template <typename Parameters>
	static std::size_t get(const Parameters &parameters) noexcept;
};

} // namespace rexlib

#include "fixed_extent.inl"
