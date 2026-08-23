// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "reduction_kernel_traits.hpp"

#include <xmipp4/core/layout/joint_layout.hpp>

#include <cstddef>
#include <tuple>

namespace rex
{
namespace cpu
{

class loop_schedule;

/**
 * @defgroup cpu_reduction_loop Reduction loop
 *
 * The iteration scheme shared by every CPU reduction program builder.
 *
 * A reduction folds the elements of its inputs along a set of axes. The axes
 * are a parameter of the operation, so the partition of the iteration space
 * into reduced and kept axes is known when the program is built and is
 * expressed as **two layouts**: one over the axes that survive into the
 * output and one over the axes being folded away.
 *
 * Splitting the space in two, rather than using a single layout with a
 * broadcast output, is what makes the loop nest expressible. A single layout
 * sorts its axes by memory locality, which interleaves reduced and kept axes
 * in an order the operation does not control, and the traversal API only
 * slices a contiguous range of axes. Two layouts also let each half be sorted
 * and coalesced for its own traversal instead of sharing one compromise
 * ordering.
 *
 * ### Two entry points
 *
 * @ref run_reduction_vector_loop is the flat one. It owns the traversal and
 * the stride dispatch, and hands its kernel the bulk of the fold a run or a
 * strip at a time, with the strides resolved. A kernel written against
 * explicit vector types plugs in here.
 *
 * @ref run_reduction_loop builds on it through
 * @ref reduction_element_adaptor, which supplies those bulk members as loops
 * over an ordinary `combine` and, where the kernel permits, deals the run out
 * over several sub-accumulators so that the fold is not one dependent chain.
 * This is the entry point most reduction program builders should use.
 *
 * @{
 */

/**
 * @brief Run a reduction over a layout, handing the kernel the fold in bulk.
 *
 * The nest walks @p kept_layout at 1D-vector granularity, blocks each vector
 * into tiles, and for every tile folds the whole of @p reduced_layout into
 * stack local accumulators before writing the outputs once. Completing a
 * tile before moving on is what keeps the accumulators in registers or in
 * cache, removes the read-modify-write an accumulate-through-the-output
 * scheme would perform on every element, and lets the accumulator type and
 * count be unrelated to those of the outputs.
 *
 * The accumulators are seeded from the first element rather than from an
 * identity. A fold that has no neutral element is therefore expressed the
 * same way as one that has, and a reduction over a single element costs
 * nothing beyond reading it.
 *
 * ### The kernel
 *
 * Everything that happens once per output is spelled one element at a time,
 * because it happens once per output; only the bulk of the fold is handed
 * over wholesale. @p kernel is invoked as:
 * @code
 * kernel.seed    (accumulators..., inputs..., position)
 * kernel.merge   (accumulators..., others...)
 * kernel.finalize(outputs..., accumulators..., count)
 * kernel.identity(accumulators...)               // only when count is zero
 *
 * kernel.combine_run  (accumulators, inputs, strides, count, position)
 * kernel.combine_strip(accumulators, inputs, kept_strides, reduced_strides,
 *                      width, count, position)
 * @endcode
 * where `accumulators` are mutable references, `inputs` are pointers to the
 * element of each input for the current iteration and `outputs` are pointers
 * to the element of each output being completed. `position` says where in
 * the reduced space the element sits, counted from the first element folded
 * into that output; only an operation reporting a location has any use for
 * it, and the arithmetic behind it disappears for the operations that
 * ignore it. It is only meaningful when the reduced space is traversed in a
 * defined order, which @ref reduction_layout_plan arranges on request.
 *
 * The two bulk members take tuples rather than flattened packs, as
 * @ref run_elementwise_vector_loop does and for the same reason: two packs
 * side by side cannot both be deduced from one argument list.
 *
 * `combine_run` folds `count` consecutive elements of the reduced space into
 * **one** accumulator set. `accumulators` is a `std::tuple` of one pointer
 * per accumulator, `inputs` a `std::tuple` of one pointer per input placed at
 * the first element of the run, and `strides` a `std::tuple` of the resolved
 * inner strides of the reduced layout, one per input. Element `e` of the run
 * sits at `input + e*stride` and at position `position + e`.
 *
 * `combine_strip` folds a run into each of `width` consecutive accumulator
 * sets, and so is handed both layouts' strides. `accumulators` is a
 * `std::tuple` of one pointer per accumulator, each at the first accumulator
 * of its strip and walked with a stride of one; `inputs` sits at the first
 * element of the run for the first accumulator. Accumulator `j` takes element
 * `e` of the run at `input + e*reduced_stride + j*kept_stride`, at position
 * `position + e`.
 *
 * Both loops belong to the kernel here, rather than the reduced one being
 * driven from outside, because which of them runs innermost is what decides
 * whether the accumulators can live in registers: a block of them folded over
 * the length of a run is written once, where a strip walked once per element
 * of the reduced space is written every time.
 *
 * Which of the two carries the fold is decided by the layouts, not by the
 * kernel: the loop calls `combine_run` when the axis being folded is the
 * contiguous one, so that each output's run is read as the stream it is, and
 * `combine_strip` when the surviving axis is, so that consecutive outputs and
 * the elements feeding them are both walked contiguously. Taking the other
 * one in either case would turn a stream into a gather, which costs an order
 * of magnitude. A kernel must therefore supply both.
 *
 * `merge` folds one set of accumulators into another. This loop calls it when
 * a reduced vector is split across workers and their partial accumulators
 * have to be combined; a kernel may also have it called by whatever supplies
 * `combine_run`, which is what @ref reduction_element_adaptor does with the
 * lanes it deals a run out over.
 *
 * The accumulator types come from `Kernel::accumulators<Outputs, Inputs>::type`
 * and are unrelated to the number of outputs: a kernel may keep two
 * accumulators and write one output, or the other way round.
 *
 * @tparam Kernel The vector reduction kernel.
 * @tparam Outs Element types of the outputs.
 * @tparam Ins Element types of the inputs.
 * @param kernel The kernel describing the fold.
 * @param kept_layout Layout over the axes that survive into the output. Its
 * operands are the inputs, in order, followed by the outputs, in order.
 * @param reduced_layout Layout over the axes being folded away. Its operands
 * are the inputs, in order.
 * @param reduction_count Number of elements folded into each output, that is
 * the product of the reduced extents. Passed to `finalize` for the operations
 * whose result depends on it, such as an average.
 * @param outputs Base pointer of each output.
 * @param inputs Base pointer of each input.
 *
 * @throws std::invalid_argument When the reduction is over no elements and
 * @p kernel supplies no identity.
 *
 * @warning Inherits the stride specialization cost of
 * @ref dispatch_inner_loop_strides on both layouts: @p kernel is instantiated
 * for up to `3^(2N)` stride combinations, where `N` is the number of inputs.
 * The outputs are deliberately left out of the dispatch, so the cost does not
 * grow with the number of outputs.
 *
 * @see run_reduction_loop
 * @see reduction_element_adaptor
 */
template <typename Kernel, typename... Outs, typename... Ins>
void run_reduction_vector_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs
);

/**
 * @brief Hand the kernel the fold in bulk, spreading the work over the
 * threads a schedule names.
 *
 * As the unscheduled overload, with the same two strategies
 * @ref run_reduction_loop describes.
 *
 * @tparam Kernel The vector reduction kernel.
 * @tparam Outs Element types of the outputs.
 * @tparam Ins Element types of the inputs.
 * @param kernel The kernel describing the fold.
 * @param kept_layout Layout over the axes that survive.
 * @param reduced_layout Layout over the axes being folded away.
 * @param reduction_count How many elements each output folds.
 * @param outputs Base pointer of each output.
 * @param inputs Base pointer of each input.
 * @param schedule The threads to spread the loop over.
 *
 * @throws std::invalid_argument if the reduction is over no elements and the
 * kernel has no identity.
 *
 * @see run_reduction_vector_loop
 * @see loop_schedule
 */
template <typename Kernel, typename... Outs, typename... Ins>
void run_reduction_vector_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	const loop_schedule &schedule
);

/**
 * @brief Run a reduction over a layout, folding the reduced axes into an
 * accumulator per output element one element at a time.
 *
 * @ref run_reduction_vector_loop with its kernel wrapped in a
 * @ref reduction_element_adaptor: the traversal, the tiling and the stride
 * dispatch are the same, and the kernel answers about one element at a time
 * instead of about a whole run. This is the entry point most CPU reduction
 * program builders should use.
 *
 * @p kernel is invoked as:
 * @code
 * kernel.seed    (accumulators..., inputs..., position)
 * kernel.combine (accumulators..., inputs..., position)
 * kernel.merge   (accumulators..., others...)
 * kernel.finalize(outputs..., accumulators..., count)
 * kernel.identity(accumulators...)               // only when count is zero
 * @endcode
 * with the meanings @ref run_reduction_vector_loop gives them.
 *
 * A kernel that additionally declares
 * @code
 * static constexpr bool reassociable_fold = true;
 * @endcode
 * has its runs dealt out over several sub-accumulators rather than folded
 * into one, which is what keeps a fold along the contiguous axis from being
 * a single chain of dependent operations. See @ref has_reassociable_fold for
 * what that commits to, and @ref reduction_element_adaptor for how it is
 * done.
 *
 * @tparam Kernel The element reduction kernel.
 * @tparam Outs Element types of the outputs.
 * @tparam Ins Element types of the inputs.
 * @param kernel The kernel describing the fold.
 * @param kept_layout Layout over the axes that survive into the output. Its
 * operands are the inputs, in order, followed by the outputs, in order.
 * @param reduced_layout Layout over the axes being folded away. Its operands
 * are the inputs, in order.
 * @param reduction_count Number of elements folded into each output.
 * @param outputs Base pointer of each output.
 * @param inputs Base pointer of each input.
 *
 * @throws std::invalid_argument When the reduction is over no elements and
 * @p kernel supplies no identity.
 *
 * @warning Inherits the stride specialization cost of
 * @ref run_reduction_vector_loop.
 *
 * @see run_reduction_vector_loop
 * @see reduction_element_adaptor
 */
template <typename Kernel, typename... Outs, typename... Ins>
void run_reduction_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs
);

/**
 * @brief Fold the inputs into the outputs, spreading the surviving space over
 * the threads a schedule names.
 *
 * As the unscheduled overload, but the surviving space is cut into contiguous
 * ranges and each is folded by one thread. Every output is written by exactly
 * one of them, and each is folded in the order it would be folded serially,
 * so the answer is the same bit for bit whatever the number of threads and
 * whatever the kernel. `merge` is not needed here and is not called.
 *
 * The threads divide the outputs, so a reduction with few of them and a deep
 * fold behind each, of which a reduction to a scalar is the extreme, gains
 * nothing from this. Such a shape has the fold itself split instead, each
 * worker taking a contiguous slice of the reduced space and their partials
 * being merged at the end. That path does call `merge`, and it reassociates
 * the fold, so a sum of floating point values stops being bit for bit what
 * one thread computes.
 *
 * The kernel is invoked concurrently on one and the same object. Every
 * accumulator and cursor a chunk works with belongs to that chunk, so a
 * kernel holding no state of its own, which is what every one of them holds,
 * needs no thought.
 *
 * @tparam Kernel The element reduction kernel.
 * @tparam Outs Element types of the outputs.
 * @tparam Ins Element types of the inputs.
 * @param kernel The kernel describing the fold.
 * @param kept_layout Layout over the axes that survive.
 * @param reduced_layout Layout over the axes being folded away.
 * @param reduction_count How many elements each output folds.
 * @param outputs Base pointer of each output.
 * @param inputs Base pointer of each input.
 * @param schedule The threads to spread the loop over.
 *
 * @throws std::invalid_argument if the reduction is over no elements and the
 * kernel has no identity.
 *
 * @see run_reduction_loop
 * @see loop_schedule
 */
template <typename Kernel, typename... Outs, typename... Ins>
void run_reduction_loop(
	const Kernel &kernel,
	const joint_layout &kept_layout,
	const joint_layout &reduced_layout,
	std::size_t reduction_count,
	const std::tuple<Outs*...> &outputs,
	const std::tuple<const Ins*...> &inputs,
	const loop_schedule &schedule
);

/** @} */

} // namespace cpu
} // namespace rex

#include "reduction_loop.inl"
