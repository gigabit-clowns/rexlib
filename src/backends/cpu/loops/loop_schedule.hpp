// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/backends/cpu/thread_pool.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief How a loop's iterations are spread over the threads running it.
 *
 * Small enough to pass by value, carrying the pool a loop may use and the
 * smallest slice of it worth a thread of its own. A default constructed
 * schedule runs everything on the calling thread, so a loop keeps working
 * without one and the tests driving loops directly do not have to name a pool
 * they do not care about.
 *
 * The loop entry points end in a variadic pack of operand pointers, leaving
 * no room for a trailing grain parameter, which is the other reason the pool
 * and the grain travel together as one value: it slots in ahead of the pack.
 */
class loop_schedule
{
public:
	/**
	 * @brief Construct a schedule that runs everything on the calling thread.
	 */
	loop_schedule() noexcept;

	/**
	 * @brief Construct a schedule over a pool.
	 *
	 * @param pool The pool to spread loops over.
	 * @param grain_size Smallest number of iterations worth a thread of its
	 * own. See grain_for_cost in parallel_grain.hpp.
	 */
	loop_schedule(thread_pool &pool, std::size_t grain_size) noexcept;

	/**
	 * @brief Get how many threads a loop would be spread over.
	 *
	 * One when there is no pool. A caller deciding how to divide its work
	 * before dividing it asks this, so that the decision and the division
	 * cannot disagree.
	 *
	 * @return std::size_t The number of threads. Never zero.
	 */
	std::size_t get_concurrency() const noexcept;

	/**
	 * @brief Get the smallest slice worth a thread of its own.
	 *
	 * @return std::size_t The grain, in iterations.
	 */
	std::size_t get_grain_size() const noexcept;

	/**
	 * @brief Get this schedule with a different grain.
	 *
	 * A functor driving more than one loop, as a reduction does, states what
	 * each of them costs without having to respell the pool.
	 *
	 * @param grain_size The grain of the new schedule.
	 * @return loop_schedule The same pool, at the given grain.
	 */
	loop_schedule with_grain(std::size_t grain_size) const noexcept;

	/**
	 * @brief Get this schedule with its threads given up.
	 *
	 * What a loop hands to whatever it calls from inside its own body: the
	 * threads have been spent on the outer loop, and saying so is how the
	 * inner one knows, rather than by asking the pool whether it is nested.
	 * The pool would run such a loop inline regardless, so this only makes
	 * the decisions taken before splitting agree with what happens after.
	 *
	 * @return loop_schedule The same grain, on the calling thread alone.
	 */
	loop_schedule serial() const noexcept;

	/**
	 * @brief Run a loop over `[0, count)`, splitting it where it pays.
	 *
	 * @tparam Body Functor invoked as `body(begin, end)`, once with the whole
	 * range when there is nothing to split. It is invoked concurrently on one
	 * and the same object; see @ref thread_pool::run.
	 * @param count Number of iterations.
	 * @param body The loop body.
	 */
	template <typename Body>
	void run(std::size_t count, const Body &body) const;

private:
	thread_pool *m_pool;
	std::size_t m_grain_size;
};

} // namespace cpu
} // namespace rexlib

#include "loop_schedule.inl"
