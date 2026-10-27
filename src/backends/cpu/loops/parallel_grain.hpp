// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief The elementwise work below which threading a loop does not pay.
 *
 * The hand-off, the cache line the two threads either side of a chunk
 * boundary share, and the join all have to be paid for before a second thread
 * is worth having.
 *
 * Read once from the REXLIB_PARALLEL_GRAIN_SIZE environment variable,
 * defaulting to the build knob of the same name. It is settable at run time
 * because a test suite works on operands orders of magnitude below any
 * sensible default, and would otherwise never reach the threaded paths at
 * all.
 *
 * @return std::size_t The grain, in elementwise operations. Never zero.
 */
std::size_t get_parallel_grain_size() noexcept;

/**
 * @brief The smallest number of iterations worth a thread of its own.
 *
 * The budget is stated in elementwise operations, so a loop whose iteration
 * costs more than one of them divides it down: a reduction's surviving
 * element costs as many as it folds, a linalg batch element costs a whole
 * matrix product, and both arrive at a grain in their own iterations without
 * a unit having to be invented for either.
 *
 * @param cost_per_iteration What one iteration costs, in elementwise
 * operations. Treated as one when zero.
 * @return std::size_t The grain, in iterations. Never zero.
 */
std::size_t grain_for_cost(std::size_t cost_per_iteration) noexcept;

/**
 * @brief Whether to share out the inner space of a nested loop, rather
 * than its outer space.
 *
 * The outer space is preferred wherever it has the parallelism to spare:
 * splitting it keeps every inner loop on one thread, so its answer is what
 * one thread computes down to the last bit. Twice the worker count rather
 * than once, so that a ragged finish has somewhere to go.
 *
 * Only when the outer space has too few iterations for that is the inner
 * space worth splitting. Below one grain nothing is worth splitting, and
 * the schedule refuses on its own.
 *
 * @param outer_count The iterations of the outer space.
 * @param inner_count The iterations of the inner space, for each outer one.
 * @param workers The number of threads available.
 * @return true The inner space is worth splitting.
 * @return false The outer space is to be split, or nothing is.
 */
bool should_split_inner_space(
	std::size_t outer_count,
	std::size_t inner_count,
	std::size_t workers
) noexcept;

} // namespace cpu
} // namespace rexlib
