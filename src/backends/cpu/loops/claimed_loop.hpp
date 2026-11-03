// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/backends/cpu/thread_pool.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Run a loop whose iterations are claimed one at a time by the
 * threads of a pool.
 *
 * Every participant of the pool takes the next unclaimed iteration until
 * none is left, so iterations of uneven cost balance out. Which thread runs
 * an iteration depends on timing; the set of iterations run does not.
 *
 * @tparam Body Functor invoked as `body(index)` for every index in
 * `[0, count)`.
 * @param pool The pool to spread the loop over.
 * @param count Number of iterations.
 * @param body The loop body. Invoked concurrently on one and the same
 * object.
 *
 * @throws Whatever @p body throws. See @ref thread_pool::run.
 */
template <typename Body>
void run_claimed_loop(thread_pool &pool, std::size_t count, const Body &body);

} // namespace cpu
} // namespace rexlib

#include "claimed_loop.inl"
