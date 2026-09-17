// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/element_index_tags.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

class operand_signature;

namespace cpu
{

/**
 * @brief The iteration a reduction is run with.
 *
 * A reduction walks two spaces: the axes that survive into the output, once
 * per output element, and the axes being folded away, once per element being
 * folded. This class holds one layout for each, plus the number of elements
 * that reach every output.
 *
 * Keeping the two spaces apart is what lets the loop complete one output at a
 * time, and lets each space be ordered for its own traversal instead of
 * sharing one compromise ordering. A single layout with the output broadcast
 * over the reduced axes describes the same iteration, but sorts all the axes
 * together, which leaves the reduced ones interleaved with the kept ones in
 * an order the operation does not control.
 *
 * @see run_reduction_loop
 */
class reduction_layout_plan
{
public:
	/**
	 * @brief Plan the iteration of a reduction over a set of axes.
	 *
	 * The inputs are broadcast together to obtain the iteration space,
	 * exactly as the reduction shape policy does when deducing the output
	 * shape, and the axes then split that space in two.
	 *
	 * The inputs are added to the kept layout before the outputs, inverting
	 * the order an elementwise operation uses. The first operand decides the
	 * axis ordering, and a reduction traverses its inputs once per folded
	 * element for every single output write, so the inputs are the operands
	 * whose locality is worth optimising.
	 *
	 * @param output_signatures Signatures of the outputs. All of them carry
	 * the shape the policy deduced, so they all belong to the kept layout.
	 * @param input_signatures Signatures of the inputs. At least one is
	 * needed.
	 * @param axes The axes being reduced, ascending and without repetitions.
	 * @param keep_dimensions Whether the outputs kept the reduced axes with
	 * an extent of one instead of dropping them.
	 * @param indexing No index operand is added to either layout.
	 *
	 * @throws std::invalid_argument When there is no input, when an output
	 * does not have the rank the axes and @p keep_dimensions imply, or when
	 * the inputs cannot be broadcast together.
	 * @throws std::out_of_range When an axis is not within the broadcast
	 * rank.
	 */
	reduction_layout_plan(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		span<const std::size_t> axes,
		bool keep_dimensions,
		no_index_tag indexing = no_index_tag()
	);

	/**
	 * @brief Plan the iteration of a reduction handing its kernel the linear
	 * index of every element within the reduced space.
	 *
	 * As the overload taking @ref no_index_tag, with the linear index operand
	 * added to the reduced layout after the inputs. The reduced axes, in
	 * ascending order, are the axes of its index space.
	 *
	 * @param output_signatures Signatures of the outputs.
	 * @param input_signatures Signatures of the inputs. At least one is
	 * needed.
	 * @param axes The axes being reduced, ascending and without repetitions.
	 * @param keep_dimensions Whether the outputs kept the reduced axes.
	 * @param indexing Selects the linear index.
	 *
	 * @throws std::invalid_argument When there is no input, when an output
	 * does not have the rank the axes and @p keep_dimensions imply, or when
	 * the inputs cannot be broadcast together.
	 * @throws std::out_of_range When an axis is not within the broadcast
	 * rank.
	 */
	reduction_layout_plan(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		span<const std::size_t> axes,
		bool keep_dimensions,
		linear_index_tag indexing
	);

	/**
	 * @brief Plan the iteration of a reduction handing its kernel the
	 * coordinates of every element within the reduced space.
	 *
	 * As the overload taking @ref no_index_tag, with one index operand per
	 * reduced axis added to the reduced layout after the inputs, in ascending
	 * axis order.
	 *
	 * @param output_signatures Signatures of the outputs.
	 * @param input_signatures Signatures of the inputs. At least one is
	 * needed.
	 * @param axes The axes being reduced, ascending and without repetitions.
	 * @param keep_dimensions Whether the outputs kept the reduced axes.
	 * @param indexing Selects the coordinates.
	 *
	 * @throws std::invalid_argument When there is no input, when an output
	 * does not have the rank the axes and @p keep_dimensions imply, or when
	 * the inputs cannot be broadcast together.
	 * @throws std::out_of_range When an axis is not within the broadcast
	 * rank.
	 */
	reduction_layout_plan(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		span<const std::size_t> axes,
		bool keep_dimensions,
		multidimensional_index_tag indexing
	);

	reduction_layout_plan(const reduction_layout_plan &other) = delete;
	reduction_layout_plan(reduction_layout_plan &&other) noexcept = default;
	~reduction_layout_plan() = default;

	reduction_layout_plan&
	operator=(const reduction_layout_plan &other) = delete;
	reduction_layout_plan&
	operator=(reduction_layout_plan &&other) noexcept = default;

	/**
	 * @brief Get the layout over the axes surviving into the output.
	 *
	 * Its operands are the inputs, in order, followed by the outputs, in
	 * order.
	 *
	 * @return const joint_layout& The kept layout.
	 */
	const joint_layout& get_kept_layout() const noexcept;

	/**
	 * @brief Get the layout over the axes being folded away.
	 *
	 * Its axes are sorted for locality and merged where possible.
	 *
	 * Its operands are the inputs, in order, followed by the index operands
	 * the plan was asked for. The operand offsets belong to the kept layout
	 * alone, so this one starts every operand at zero and only ever displaces
	 * the pointer the kept layout has already placed.
	 *
	 * @return const joint_layout& The reduced layout.
	 */
	const joint_layout& get_reduced_layout() const noexcept;

	/**
	 * @brief Get the number of elements folded into each output element.
	 *
	 * The product of the reduced extents, which is one when no axis is
	 * reduced and zero when one of them is empty.
	 *
	 * @return std::size_t The number of elements.
	 */
	std::size_t get_reduction_count() const noexcept;

private:
	template <typename Indexing>
	void plan(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		span<const std::size_t> axes,
		bool keep_dimensions,
		Indexing indexing
	);

	joint_layout m_kept_layout;
	joint_layout m_reduced_layout;
	std::size_t m_reduction_count;
};

} // namespace cpu
} // namespace rexlib
