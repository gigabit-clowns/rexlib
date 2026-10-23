// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace ops
{

/**
 * @brief Accessor reading the axes an operation acts along.
 *
 * A rule element that depends on this value names the accessor. It reads
 * any parameter type with a `get_axes()` member.
 */
struct axes_of
{
	/**
	 * @brief Read the value.
	 *
	 * @tparam Parameters The parameter type of the operation.
	 * @param parameters The parameters of the operation instance.
	 * @return span<const std::size_t> The axes.
	 */
	template <typename Parameters>
	static span<const std::size_t> get(const Parameters &parameters) noexcept;
};

} // namespace ops
} // namespace rexlib

#include "axes_of.inl"
