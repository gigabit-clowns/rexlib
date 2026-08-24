// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "linalg_operand_core.hpp"

#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/layout/joint_layout.hpp>
#include <rex/core/span.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Splits a matmul/matvec/vecmat/cross operand into a batch and a
 * core.
 *
 * Every one of these operations walks broadcastable "batch" axes plus a
 * small, fixed number of "core" axes per operand that a single kernel call
 * consumes whole (two for a matrix operand, one for a vector operand or for
 * cross's named axis). This class performs that split once, up front, so the
 * program builder only has to drive an ordinary elementwise-shaped loop over
 * the batch and hand the core, raw, to the kernel.
 *
 * @see run_elementwise_loop
 * @see linalg_operand_core
 */
class linalg_core_layout_plan
{
public:
	/**
	 * @brief Plan operands whose core axes are their trailing axes.
	 *
	 * The output's own extents, minus its trailing @p output_core_rank
	 * axes, are trusted as the canonical batch shape: the shape policy
	 * already broadcast and validated it when it deduced the output shape.
	 * Every operand's remaining batch prefix is then handed to a
	 * joint_layout_builder, which broadcasts it to that shape.
	 *
	 * @param output_signatures Exactly one output signature.
	 * @param input_signatures Exactly two input signatures (left, right).
	 * @param output_core_rank Number of trailing axes of the output that
	 * are core.
	 * @param left_core_rank Number of trailing axes of `left` that are
	 * core.
	 * @param right_core_rank Number of trailing axes of `right` that are
	 * core.
	 *
	 * @throws std::invalid_argument If an operand's rank is smaller than
	 * its declared core rank.
	 */
	static linalg_core_layout_plan for_trailing_core(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		std::size_t output_core_rank,
		std::size_t left_core_rank,
		std::size_t right_core_rank
	);

	/**
	 * @brief Plan operands sharing one named core axis (cross).
	 *
	 * Every operand's full layout is broadcast to the output's full
	 * extents first (an ordinary elementwise-style broadcast, since the
	 * named axis is a broadcast axis pinned to a fixed extent, not an
	 * asymmetric contraction), and the axis at @p core_axis is then pulled
	 * out of the broadcast result as the core.
	 *
	 * @param core_axis Index into the canonical (post-broadcast) rank,
	 * the same index for every operand.
	 */
	static linalg_core_layout_plan for_named_axis(
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		std::size_t core_axis
	);

	linalg_core_layout_plan(
		joint_layout batch_layout,
		const linalg_operand_core &output_core,
		const linalg_operand_core &left_core,
		const linalg_operand_core &right_core
	) noexcept;

	/**
	 * @brief The batch iteration layout.
	 *
	 * Operands in order: output, left, right.
	 *
	 * @return const joint_layout& The batch layout.
	 */
	const joint_layout& get_batch_layout() const noexcept;

	const linalg_operand_core& get_output_core() const noexcept;
	const linalg_operand_core& get_left_core() const noexcept;
	const linalg_operand_core& get_right_core() const noexcept;

	/**
	 * @brief The arithmetic one batch element costs.
	 *
	 * Multiplications, which is the term that decides: a matrix product costs
	 * the product of the left operand's extents by the width of the right
	 * one, and a vector operand contributes nothing to that width because it
	 * has none.
	 *
	 * It is only ever divided into a grain, so an order of magnitude is all
	 * it has to be right by. It lives here rather than on the builder because
	 * cross builds its functor by hand and has to arrive at the same number.
	 *
	 * @return std::size_t The cost, in elementwise operations. Never zero.
	 */
	std::size_t get_core_cost() const noexcept;

private:
	joint_layout m_batch_layout;
	linalg_operand_core m_output_core;
	linalg_operand_core m_left_core;
	linalg_operand_core m_right_core;
};

} // namespace cpu
} // namespace rexlib
