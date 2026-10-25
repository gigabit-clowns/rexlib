// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

/**
 * @brief Origin of the value of a typing rule pivot.
 */
enum class type_pivot_source
{
	/**
	 * @brief Fixed by the data type of an input operand.
	 */
	input,

	/**
	 * @brief Free. Adopted from the user supplied output operand, if any.
	 */
	output,

	/**
	 * @brief Supplied by the operation instance at runtime.
	 */
	operation
};

} // namespace rexlib
