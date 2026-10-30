// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/element_index_tags.hpp>
#include <backends/cpu/plans/named_core_frame.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

class operand_descriptor;

namespace cpu
{

/**
 * @brief The iteration of an operation that reduces the core of every batch
 * element to statistics, and then maps that core with them.
 *
 * The core axes are the named ones, and the batch axes are the rest. The
 * batch layout visits every batch element once. For each of them, the
 * reductions walk the scalar layout and the reduced layout, and the map
 * walks the map layout. The operand offsets belong to the batch layout
 * alone: every other layout starts its operands at zero and displaces the
 * pointers the batch layout has placed.
 *
 * Inputs without core axes take part in the batch layout only, so that a
 * map reads their value once per batch element.
 *
 * The map may be handed a counter: the row-major position of an element
 * among the elements of the result that share the values of every input
 * without core axes. The counter is split in two operands, one in the batch
 * layout and one in the map layout, whose offsets add up to it.
 */
class batch_reduce_map_plan
{
public:
	/**
	 * @brief Plan the iteration over a set of named core axes.
	 *
	 * @tparam ReductionIndexing The index operands added to the reduced
	 * layout: no_index_tag, linear_index_tag or multidimensional_index_tag,
	 * over the core axes in ascending order.
	 * @tparam MapIndexing The counter: no_index_tag for none, or
	 * linear_index_tag.
	 * @param output_descriptors Descriptors of the outputs, each of the
	 * shape of the result. At least one is needed.
	 * @param input_descriptors Descriptors of the inputs.
	 * @param input_cores The core of every input, as the shape rule declares
	 * it. An empty core marks an input without core axes.
	 * @param named_axes The core axes, in any order.
	 * @param statistic_count The number of statistics each reduction
	 * produces.
	 * @param reduction_indexing Selects the index operands of the reduced
	 * layout.
	 * @param map_indexing Selects the counter.
	 * @throws std::invalid_argument When there is no output, when the cores
	 * do not match the inputs, when no input carries the core axes, or when
	 * an operand does not broadcast to the result.
	 * @throws std::out_of_range When a named axis is not within the result.
	 */
	template <typename ReductionIndexing, typename MapIndexing>
	batch_reduce_map_plan(
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		span<const core_dimensions_descriptor> input_cores,
		span<const std::size_t> named_axes,
		std::size_t statistic_count,
		ReductionIndexing reduction_indexing,
		MapIndexing map_indexing
	);

	batch_reduce_map_plan(const batch_reduce_map_plan &other) = delete;
	batch_reduce_map_plan(batch_reduce_map_plan &&other) noexcept = default;
	~batch_reduce_map_plan() = default;

	batch_reduce_map_plan&
	operator=(const batch_reduce_map_plan &other) = delete;
	batch_reduce_map_plan&
	operator=(batch_reduce_map_plan &&other) noexcept = default;

	/**
	 * @brief Get the core axes and the batch axes of the result.
	 *
	 * @return const named_core_frame& The frame.
	 */
	const named_core_frame& get_frame() const noexcept;

	/**
	 * @brief Get the layout over the batch axes.
	 *
	 * Its operands are the inputs, in order, followed by the outputs, in
	 * order, followed by the batch part of the counter when there is one.
	 *
	 * @return const joint_layout& The batch layout.
	 */
	const joint_layout& get_batch_layout() const noexcept;

	/**
	 * @brief Get the layout of rank 0 a reduction writes its statistics
	 * through.
	 *
	 * Its operands are the inputs carrying the core axes, in order,
	 * followed by one operand per statistic.
	 *
	 * @return const joint_layout& The scalar layout.
	 */
	const joint_layout& get_scalar_layout() const noexcept;

	/**
	 * @brief Get the layout a reduction folds the core axes through.
	 *
	 * Its operands are the inputs carrying the core axes, in order,
	 * followed by the index operands the plan was asked for.
	 *
	 * @return const joint_layout& The reduced layout.
	 */
	const joint_layout& get_reduced_layout() const noexcept;

	/**
	 * @brief Get the layout the map walks the core axes through.
	 *
	 * Its operands are the outputs, in order, followed by the inputs
	 * carrying the core axes, in order, followed by the core part of the
	 * counter when there is one.
	 *
	 * @return const joint_layout& The map layout.
	 */
	const joint_layout& get_map_layout() const noexcept;

	/**
	 * @brief Get the number of batch elements.
	 *
	 * @return std::size_t The product of the batch extents.
	 */
	std::size_t get_batch_count() const noexcept;

	/**
	 * @brief Get the number of elements in the core of a batch element.
	 *
	 * @return std::size_t The product of the core extents.
	 */
	std::size_t get_core_count() const noexcept;

private:
	named_core_frame m_frame;
	joint_layout m_batch_layout;
	joint_layout m_scalar_layout;
	joint_layout m_reduced_layout;
	joint_layout m_map_layout;
	std::size_t m_batch_count;
	std::size_t m_core_count;
};

} // namespace cpu
} // namespace rexlib
