// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Accessor reading whether an operation keeps the axes it removes
 * with an extent of one.
 *
 * A rule element that depends on this value names the accessor. It reads
 * any parameter type with a `get_keep_dimensions()` member.
 */
struct keep_dimensions_of
{
	/**
	 * @brief Read the value.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return bool Whether the axes are kept.
	 */
	template <typename Parameters>
	static bool get(const Parameters &parameters) noexcept;
};

} // namespace ops
} // namespace rexlib

#include "keep_dimensions_of.inl"
