// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "thread_pool.hpp"

#include <cstddef>

namespace xmipp4
{
namespace cpu
{

/**
 * @brief Run a loop, splitting it across a pool.
 *
 * Type-safe wrapper over @ref thread_pool::run. @p body is referred to rather
 * than copied, so nothing is allocated: the object lives in the caller's
 * frame, which @ref thread_pool::run does not outlive.
 *
 * The body is invoked through a const reference, so a mutable lambda does not
 * compile here. That is deliberate: the body runs concurrently on one and the
 * same object, so state a chunk keeps to itself has to be created inside the
 * body rather than captured by it. Mutating through what the body captures by
 * reference is unaffected, and is what every caller does.
 *
 * A free function rather than a member so that @ref thread_pool is a class
 * whose every method has a body in one translation unit, and the only thing
 * needing to be a template lives on its own.
 *
 * @tparam Body Functor invoked as `body(begin, end)` with the half-open range
 * of iterations one chunk covers.
 * @param pool The pool to spread the loop over.
 * @param count Number of iterations.
 * @param grain_size Smallest number of iterations worth handing to a thread of
 * its own. Treated as one when zero.
 * @param body The loop body.
 *
 * @throws Whatever @p body throws. See @ref thread_pool::run.
 *
 * @see thread_pool::run
 */
template <typename Body>
void parallel_for(
	thread_pool &pool,
	std::size_t count,
	std::size_t grain_size,
	const Body &body
);

} // namespace cpu
} // namespace xmipp4

#include "parallel_for.inl"
