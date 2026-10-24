// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

/**
 * @brief Origin of the value of a shape rule extent variable.
 */
enum class extent_pivot_source
{
	/**
	 * @brief Bound by the input axes that name the variable.
	 */
	inputs,

	/**
	 * @brief Supplied by the operation instance.
	 */
	operation
};

} // namespace rexlib
