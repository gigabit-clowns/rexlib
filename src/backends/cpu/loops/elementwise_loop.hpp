// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/layout/joint_layout.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

class loop_schedule;

/**
 * @brief Apply an inner loop to every 1D vector of a multidimensional layout.
 *
 * Iterates over the outer dimensions of the layout, invoking the inner loop
 * once per 1D vector (the innermost dimension of the layout). On each
 * invocation the operand pointers are advanced to the start of the current
 * vector and the number of elements in it is provided; per-element strides
 * within the vector are left for the inner loop to handle.
 *
 * @tparam InnerLoop Functor to be dispatched for 1D vectors. Must have a
 * signature accepting `(Pointers... operands, std::size_t count)`.
 * @tparam Pointers CV-qualified pointers. There must be one per operand in the
 * layout.
 * @param inner_loop The functor to be invoked for each 1D vector.
 * @param layout Access layout used for iterating over the operands.
 * @param pointers The base pointer of each operand.
 *
 * @see run_elementwise_vector_loop
 * @see run_elementwise_loop
 */
template <typename InnerLoop, typename... Pointers>
void run_elementwise_outer_loop(
	InnerLoop &&inner_loop,
	const joint_layout &layout,
	Pointers... pointers
);

/**
 * @brief Apply an inner loop to part of a multidimensional layout.
 *
 * As @ref run_elementwise_outer_loop, but walking only the elements in
 * `[begin, end)` of the layout's iteration space, counted the way the
 * traversal counts them. Two calls covering a range between them visit
 * exactly what one call over the whole of it visits, in the same order per
 * element, which is what lets an elementwise loop be shared out over threads.
 *
 * The range does not have to fall on the boundaries of the innermost axis. A
 * range starting or ending inside a vector simply hands the inner loop a
 * shorter one, beginning where it begins: the count says how many elements
 * there are, never that they start an axis.
 *
 * @tparam InnerLoop Functor to be dispatched for 1D vectors. Must have a
 * signature accepting `(Pointers... operands, std::size_t count)`.
 * @tparam Pointers CV-qualified pointers. There must be one per operand in
 * the layout.
 * @param inner_loop The functor to be invoked for each 1D vector.
 * @param layout Access layout used for iterating over the operands.
 * @param begin First element of the iteration space to visit.
 * @param end Past-the-end element of the iteration space to visit. Must not
 * exceed the number of elements the layout holds.
 * @param pointers The base pointer of each operand.
 *
 * @see run_elementwise_outer_loop
 */
template <typename InnerLoop, typename... Pointers>
void run_elementwise_outer_loop_range(
	InnerLoop &&inner_loop,
	const joint_layout &layout,
	std::size_t begin,
	std::size_t end,
	Pointers... pointers
);

/**
 * @brief Run an elementwise loop over a layout at 1D-vector granularity,
 * statically dispatching each operand's inner-loop stride.
 *
 * Composes @ref dispatch_inner_loop_strides and
 * @ref run_elementwise_outer_loop: each operand's inner stride is resolved to
 * a stride tag once, then the outer dimensions of @p layout are iterated,
 * invoking @p kernel once per 1D vector. This is the flat building block
 * shared by CPU elementwise operation program builders.
 *
 * @p kernel operates on a whole 1D vector at a time (with its per-operand
 * strides statically resolved); it does not itself iterate the individual
 * elements. For a kernel that applies an operation once per element, use
 * @ref run_elementwise_loop instead, which builds on this function.
 *
 * The kernel is invoked as:
 * @code
 * kernel(pointers, strides, count)
 * @endcode
 * where `pointers` is a `std::tuple` of the operand pointers advanced to the
 * start of the current 1D vector, `strides` is a `std::tuple` of the
 * statically resolved inner strides (one per operand, in the same operand
 * order as `pointers`; each element is a `contiguous_stride_tag`, a
 * `broadcasting_stride_tag` or a runtime `std::ptrdiff_t`) and `count` is the
 * number of elements in the vector.
 *
 * @tparam Kernel Functor invoked per 1D vector. Must accept
 * `(std::tuple<Pointers...>, std::tuple<Strides...>, std::size_t)`.
 * @tparam Pointers CV-qualified operand pointers, one per operand.
 * @param kernel The functor to be invoked for each 1D vector.
 * @param layout Access layout used both for stride dispatch and for iterating
 * over the operands.
 * @param pointers The base pointer of each operand.
 *
 * @warning Inherits the stride specialization cost of
 * @ref dispatch_inner_loop_strides: @p kernel is instantiated for up to `3^N`
 * stride combinations, where `N` is the number of operands.
 *
 * @see dispatch_inner_loop_strides
 * @see run_elementwise_outer_loop
 * @see run_elementwise_loop
 */
template <typename Kernel, typename... Pointers>
void run_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	Pointers... pointers
);

/**
 * @brief Run an elementwise loop over a layout at 1D-vector granularity,
 * spreading it over the threads a schedule names.
 *
 * As the unscheduled overload, but the iteration space is cut into contiguous
 * ranges and each is walked by @ref run_elementwise_outer_loop_range. Every
 * element is visited exactly once between them, and the kernel sees the same
 * vectors it would see serially, except that one falling across a chunk
 * boundary reaches it as two shorter ones.
 *
 * The kernel is invoked concurrently on one and the same object, so any state
 * a chunk needs to itself has to be created inside it.
 *
 * The stride dispatch happens once per chunk rather than once per loop, which
 * is what keeps the schedule's body from being instantiated once per stride
 * combination.
 *
 * @tparam Kernel Functor invoked per 1D vector. Must accept
 * `(std::tuple<Pointers...>, std::tuple<Strides...>, std::size_t)`.
 * @tparam Pointers CV-qualified operand pointers, one per operand.
 * @param kernel The functor to be invoked for each 1D vector.
 * @param layout Access layout used both for stride dispatch and for iterating
 * over the operands.
 * @param schedule The threads to spread the loop over, and the smallest slice
 * worth handing to one of them.
 * @param pointers The base pointer of each operand.
 *
 * @see run_elementwise_vector_loop
 * @see loop_schedule
 */
template <typename Kernel, typename... Pointers>
void run_elementwise_vector_loop(
	const Kernel &kernel,
	const joint_layout &layout,
	const loop_schedule &schedule,
	Pointers... pointers
);

/**
 * @brief Run an elementwise loop over a layout, applying an operation once per
 * element.
 *
 * Builds on @ref run_elementwise_vector_loop by additionally applying @p op to
 * every element of each 1D vector it produces: where
 * @ref run_elementwise_vector_loop stops at vector granularity (outer-loop
 * iteration plus inner-stride dispatch, leaving the per-element application to
 * its kernel), this function also performs that per-element application. This
 * is the entry point most CPU elementwise operation program builders should
 * use.
 *
 * For each element, @p op is invoked with one pointer per operand, in the same
 * order as @p pointers:
 * @code
 * op(current0, current1, ...)
 * @endcode
 * where `currentK` points to the element of operand `K` for the current
 * iteration, i.e. its base pointer advanced by that operand's stride (for a
 * contiguous operand this is `&operandK[i]`, for a broadcasted operand it
 * stays at the first element, and for a runtime stride it is
 * `operandK + i * stride`).
 * The pointers preserve each operand's cv-qualification (mutable for outputs,
 * `const` for inputs), so @p op itself decides which operands it writes to;
 * this function draws no distinction between inputs and outputs and imposes no
 * ordering between them, so N-input/M-output operations are expressed the same
 * way as a plain unary or binary one.
 *
 * The per-element loop is specialized on the resolved strides:
 * - an all-contiguous raw indexed loop for any operand count (the
 *   vectorization-friendly fast path)
 * - a catch-all strided-iterator loop for any other stride combination. When
 *   the strides are statically known (contiguous or broadcasting tags) this
 *   loop vectorizes as well as the raw one, since the iterator carries the
 *   stride as a compile-time constant; only runtime strides fall back to
 *   scalar code.
 *
 * @tparam Op Operation invoked as `op(pointers...)` with one pointer per
 * operand, in the same order as @p pointers. May be stateful.
 * @tparam Pointers CV-qualified operand pointers, one per operand.
 * @param op The operation to be applied to every element.
 * @param layout Access layout used both for stride dispatch and for iterating
 * over the operands.
 * @param pointers The base pointer of each operand.
 *
 * @see run_elementwise_vector_loop
 */
template <typename Op, typename... Pointers>
void run_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	Pointers... pointers
);

/**
 * @brief Run an elementwise loop over a layout, applying an operation once per
 * element, spread over the threads a schedule names.
 *
 * As the unscheduled overload, with the iteration space cut into contiguous
 * ranges walked in parallel. Every element is visited exactly once, and each
 * is written by exactly one thread, so an elementwise operation needs nothing
 * of its own to be safe here.
 *
 * @p op is invoked concurrently on one and the same object, so any state a
 * chunk needs to itself has to be created inside it. A stateless operation,
 * which is what every one of them is, needs no thought.
 *
 * @tparam Op Operation invoked as `op(pointers...)` with one pointer per
 * operand, in the same order as @p pointers.
 * @tparam Pointers CV-qualified operand pointers, one per operand.
 * @param op The operation to be applied to every element.
 * @param layout Access layout used both for stride dispatch and for iterating
 * over the operands.
 * @param schedule The threads to spread the loop over, and the smallest slice
 * worth handing to one of them.
 * @param pointers The base pointer of each operand.
 *
 * @see run_elementwise_loop
 * @see loop_schedule
 */
template <typename Op, typename... Pointers>
void run_elementwise_loop(
	const Op &op,
	const joint_layout &layout,
	const loop_schedule &schedule,
	Pointers... pointers
);

} // namespace cpu
} // namespace rexlib

#include "elementwise_loop.inl"
