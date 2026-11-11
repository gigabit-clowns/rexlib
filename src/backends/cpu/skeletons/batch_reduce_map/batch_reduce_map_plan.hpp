// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/indices/element_index_tags.hpp>
#include <backends/cpu/loops/layouts/named_core_frame.hpp>
#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>

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
 * The map is a batched_map_plan whose mapped inputs are the inputs carrying
 * the core of the outputs. The reductions fold the core of those same
 * inputs, at each batch element the map plan's batch layout places them
 * at, through the scalar layout and the reduced layout. Every other input
 * is read once per batch element.
 */
class batch_reduce_map_plan
{
public:
	/**
	 * @brief Plan the reductions and the map over the core of the outputs.
	 *
	 * @tparam ReductionIndexing The index operands added to the reduced
	 * layout: no_index_tag, linear_index_tag or multidimensional_index_tag,
	 * over the core axes in ascending order.
	 * @tparam MapIndexing The index of the map, as batched_map_plan takes
	 * it.
	 * @param frame The axes of the outputs, split into their core and their
	 * batch axes.
	 * @param output_descriptors Descriptors of the outputs, each of the
	 * extents of the frame.
	 * @param input_descriptors Descriptors of the inputs.
	 * @param input_cores The core of every input, as the shape rule declares
	 * it.
	 * @param core_inputs The inputs carrying the core of the outputs,
	 * ascending. At least one.
	 * @param statistic_count The number of statistics each reduction
	 * produces.
	 * @param reduction_indexing Selects the index operands of the reduced
	 * layout.
	 * @param map_indexing Selects the index of the map.
	 * @throws std::invalid_argument When no input carries the core of the
	 * outputs, or when batched_map_plan rejects the operands.
	 */
	template <typename ReductionIndexing, typename MapIndexing>
	batch_reduce_map_plan(
		named_core_frame frame,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		span<const core_dimensions_descriptor> input_cores,
		span<const std::size_t> core_inputs,
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
	 * @brief Get the plan of the map.
	 *
	 * @return const batched_map_plan& The map plan.
	 */
	const batched_map_plan& get_map_plan() const noexcept;

	/**
	 * @brief Get the layout of rank 0 a reduction writes its statistics
	 * through.
	 *
	 * Its operands are the inputs carrying the core of the outputs, in
	 * order, followed by one operand per statistic.
	 *
	 * @return const joint_layout& The scalar layout.
	 */
	const joint_layout& get_scalar_layout() const noexcept;

	/**
	 * @brief Get the layout a reduction folds the core axes through.
	 *
	 * Its operands are the inputs carrying the core of the outputs, in
	 * order, followed by the index operands the plan was asked for.
	 *
	 * @return const joint_layout& The reduced layout.
	 */
	const joint_layout& get_reduced_layout() const noexcept;

private:
	batched_map_plan m_map_plan;
	joint_layout m_scalar_layout;
	joint_layout m_reduced_layout;
};

} // namespace cpu
} // namespace rexlib
