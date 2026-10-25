// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{

/**
 * @brief Domain admitting every extent.
 *
 * @see extent_domain
 */
struct any_extent_domain
{
	/**
	 * @brief Check whether the domain admits an extent.
	 *
	 * @param extent The extent.
	 * @return bool Always true.
	 */
	static bool contains(std::size_t extent) noexcept;

	/**
	 * @brief Get what the domain admits.
	 *
	 * @return const char* The description.
	 */
	static const char* get_description() noexcept;
};

} // namespace rexlib

#include "any_extent_domain.inl"
