// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{

/**
 * @brief Transform mapping an extent onto itself.
 *
 * Models core axes that operands share, such as the inner extent of a
 * matrix multiplication.
 *
 * @see operand_axis_slot
 */
struct same_extent_transform
{
	/**
	 * @brief Apply the transform.
	 *
	 * @param extent The extent of the variable.
	 * @return std::size_t The same extent.
	 */
	static std::size_t apply(std::size_t extent) noexcept;

	/**
	 * @brief Get a fragment naming the mapping, for diagnostics.
	 *
	 * @return const char* The fragment.
	 */
	static const char* get_relation() noexcept;
};

} // namespace rexlib

#include "same_extent_transform.inl"
