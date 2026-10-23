// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "core_dimensions_descriptor.hpp"
#include "core_placement.hpp"
#include "extent_descriptor.hpp"
#include "output_batch.hpp"

#include <rexlib/core/span.hpp>

namespace rexlib
{

/**
 * @brief Type erased form of an operand_shape_rule.
 *
 * Interpreting a rule at runtime does not need any of its types, so a rule
 * is lowered to tables of plain descriptors and a single interpreter serves
 * every operation.
 *
 * @see operand_shape_rule
 */
class shape_rule_descriptor
{
public:
	/**
	 * @brief Construct a shape rule descriptor.
	 *
	 * Nothing is owned: every table must outlive the descriptor.
	 *
	 * @param extents The extent variables.
	 * @param output_cores The core axes of every output, in signature order.
	 * @param input_cores The core axes of every input, in signature order.
	 * @param placement Where the core axes lie.
	 * @param batch Which batch axes an output supplied in advance may have.
	 */
	shape_rule_descriptor(
		span<const extent_descriptor> extents,
		span<const core_dimensions_descriptor> output_cores,
		span<const core_dimensions_descriptor> input_cores,
		core_placement placement,
		output_batch batch
	) noexcept;

	/**
	 * @brief Get the extent variables.
	 *
	 * @return span<const extent_descriptor> The extent variables.
	 */
	span<const extent_descriptor> get_extents() const noexcept;

	/**
	 * @brief Get the core axes of every output.
	 *
	 * @return span<const core_dimensions_descriptor> One entry per output.
	 */
	span<const core_dimensions_descriptor> get_output_cores() const noexcept;

	/**
	 * @brief Get the core axes of every input.
	 *
	 * @return span<const core_dimensions_descriptor> One entry per input.
	 */
	span<const core_dimensions_descriptor> get_input_cores() const noexcept;

	/**
	 * @brief Get where the core axes lie.
	 *
	 * @return core_placement The placement.
	 */
	core_placement get_placement() const noexcept;

	/**
	 * @brief Get which batch axes an output supplied in advance may have.
	 *
	 * @return output_batch The output batch.
	 */
	output_batch get_output_batch() const noexcept;

private:
	span<const extent_descriptor> m_extents;
	span<const core_dimensions_descriptor> m_output_cores;
	span<const core_dimensions_descriptor> m_input_cores;
	core_placement m_placement;
	output_batch m_output_batch;
};

} // namespace rexlib

#include "shape_rule_descriptor.inl"
