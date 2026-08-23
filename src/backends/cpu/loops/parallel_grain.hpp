// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rex
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
 * Read once from the REX_PARALLEL_GRAIN_SIZE environment variable,
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

} // namespace cpu
} // namespace rex
