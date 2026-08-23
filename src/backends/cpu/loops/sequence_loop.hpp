// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace xmipp4
{
namespace cpu
{

class loop_schedule;

/**
 * @brief Write a generated value into every element of a 1D operand.
 *
 * The counterpart of @ref run_elementwise_loop for an operation with no
 * input: there is nothing to read, so what the generator is given is the
 * position of the element rather than its previous contents. That position
 * is the index into the sequence, counted from the first element whatever
 * the operand's stride, so the generator answers about the sequence and
 * never about the memory holding it.
 *
 * There is a single loop and no layout to walk because the family this
 * serves produces one dimensional results by definition. Anything of a
 * higher rank holding a sequence is a view of one of these.
 *
 * @tparam Generator Functor invoked once per element, as
 * `generator(destination, index)`, where `destination` points at the element
 * to write and `index` is its position in the sequence.
 * @tparam T Element type of the operand.
 * @param generator The functor producing the elements.
 * @param pointer Pointer to the first element of the operand.
 * @param count Number of elements to write.
 * @param stride Element step between one element and the next.
 */
template <typename Generator, typename T>
void run_sequence_loop(
	const Generator &generator,
	T *pointer,
	std::size_t count,
	std::ptrdiff_t stride
);

/**
 * @brief Write a sequence, spreading it over the threads a schedule names.
 *
 * As the unscheduled overload, but the sequence is cut into contiguous ranges
 * of its own index and each is written by one thread. The generator is always
 * handed the position within the whole sequence, never within the chunk, so a
 * cut is invisible in what gets written.
 *
 * @p generator is invoked concurrently on one and the same object, so any
 * state a chunk needs to itself has to be created inside it. A generator
 * writing an element from its index alone, which is what this family is for,
 * needs no thought.
 *
 * @tparam Generator Functor invoked as `generator(pointer, index)`.
 * @tparam T The element type.
 * @param generator The functor writing each element.
 * @param pointer The first element of the sequence.
 * @param count The number of elements.
 * @param stride The step between consecutive elements.
 * @param schedule The threads to spread the loop over, and the smallest slice
 * worth handing to one of them.
 *
 * @see run_sequence_loop
 * @see loop_schedule
 */
template <typename Generator, typename T>
void run_sequence_loop(
	const Generator &generator,
	T *pointer,
	std::size_t count,
	std::ptrdiff_t stride,
	const loop_schedule &schedule
);

} // namespace cpu
} // namespace xmipp4

#include "sequence_loop.inl"
