// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

/**
 * @brief Where the core axes of the operands lie.
 */
enum class core_placement
{
	/**
	 * @brief The last axes of every operand.
	 */
	trailing,

	/**
	 * @brief The axes the operation instance names.
	 */
	named
};

} // namespace rexlib
