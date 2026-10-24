// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

/**
 * @brief Which batch axes an output supplied in advance may have.
 */
enum class output_batch
{
	/**
	 * @brief Exactly those the inputs broadcast to.
	 */
	exact,

	/**
	 * @brief Any the inputs broadcast to, shared by every output.
	 */
	broadcast_target
};

} // namespace rexlib
