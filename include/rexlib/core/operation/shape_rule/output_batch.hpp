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
	broadcast_target,

	/**
	 * @brief Those of the input at the same position, its accumulator.
	 *
	 * Each output has the shape of its accumulator. The batch axes of all
	 * inputs broadcast together, and the broadcast batch axes an
	 * accumulator lacks, or holds with an extent of one, are reduced into
	 * it.
	 */
	accumulated
};

} // namespace rexlib
