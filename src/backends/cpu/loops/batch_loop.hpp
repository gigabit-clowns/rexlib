// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/loop_schedule.hpp>

#include <rexlib/core/layout/joint_layout.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Run a visitor once per batch element, spreading over the threads
 * either the batch elements or the work of each of them.
 *
 * The batch elements are spread wherever they are enough to occupy the
 * threads, so that one thread runs the whole work of a batch element and
 * finds its data in cache. Otherwise they are visited one after another,
 * and each visitor is handed the threads to spread its own work over. The
 * visitor is invoked as:
 * @code
 * visitor(inner, pointer0, pointer1, ..., index)
 * @endcode
 * where `inner` is the loop_schedule of the work of one batch element, the
 * pointers are placed at the batch element, and `index` is what
 * @ref run_indexed_elementwise_loop hands for @p indexing.
 *
 * @tparam Visitor Functor invoked once per batch element.
 * @tparam Indexing One of @ref no_index_tag, @ref linear_index_tag and
 * @ref multidimensional_index_tag.
 * @tparam Pointers CV-qualified operand pointers, one per operand.
 * @param visitor The functor to be invoked for each batch element.
 * @param batch_layout The layout over the batch axes. Its operands are the
 * ones @p pointers point into, followed by the index operands of
 * @p indexing.
 * @param indexing The index to hand the visitor.
 * @param schedule The threads to run over.
 * @param element_cost What the work of one batch element costs, in units of
 * one element of an elementwise loop.
 * @param inner_count The iterations of the work of one batch element that
 * may be spread over the threads, or 0 when it cannot be spread.
 * @param pointers The base pointer of each operand.
 */
template <typename Visitor, typename Indexing, typename... Pointers>
void run_batch_loop(
	const Visitor &visitor,
	const joint_layout &batch_layout,
	Indexing indexing,
	const loop_schedule &schedule,
	std::size_t element_cost,
	std::size_t inner_count,
	Pointers... pointers
);

} // namespace cpu
} // namespace rexlib

#include "batch_loop.inl"
