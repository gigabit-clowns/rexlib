// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Accessor reading the number of values an operation produces.
 *
 * A rule element that depends on this value names the accessor. It reads
 * any parameter type with a `get_count()` member.
 */
struct count_of
{
	/**
	 * @brief Read the value.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return std::size_t The count.
	 */
	template <typename Parameters>
	static std::size_t get(const Parameters &parameters) noexcept;
};

} // namespace ops
} // namespace rexlib

#include "count_of.inl"
