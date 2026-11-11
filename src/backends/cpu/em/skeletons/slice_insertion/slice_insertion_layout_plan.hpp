// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cpu
{

/**
 * @brief Splits the operands of a slice insertion into cores and batch
 * layouts.
 *
 * Every operand's trailing core axes are peeled off raw, and the batch
 * prefixes broadcast together into a frame. The frame splits in two: the
 * kept axes, which the accumulators carry, and the reduced axes, which they
 * lack or hold with an extent of one. Each kept element is one accumulator
 * that every reduced element adds into.
 *
 * The kept layout holds the inputs then the outputs, with their offsets.
 * The reduced layout holds the inputs with an offset of zero, displacing a
 * pointer the kept layout has placed; the accumulators have a stride of
 * zero there.
 *
 * @see em::insert_slices_operation
 */
class slice_insertion_layout_plan
{
public:
	/**
	 * @brief Plan the operands of an insertion.
	 *
	 * @param output_descriptors The two accumulated outputs, each with
	 * three core axes.
	 * @param input_descriptors The inputs: the two accumulators, the slices
	 * and the rest.
	 * @param input_core_ranks The number of trailing core axes of each
	 * input.
	 *
	 * @throws std::invalid_argument When an operand has fewer axes than its
	 * core, or the batch prefixes do not broadcast together.
	 */
	slice_insertion_layout_plan(
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		span<const std::size_t> input_core_ranks
	);

	/**
	 * @brief Get the extent of the volume along each axis.
	 *
	 * @return std::size_t The extent N.
	 */
	std::size_t get_volume_extent() const noexcept;

	/**
	 * @brief Get the extent of a slice along each axis.
	 *
	 * @return std::size_t The extent n.
	 */
	std::size_t get_slice_extent() const noexcept;

	/**
	 * @brief Get the core of an output.
	 *
	 * @param output Index of the output.
	 * @return const operand_core& The core.
	 */
	const operand_core& get_output_core(std::size_t output) const noexcept;

	/**
	 * @brief Get the core of an input.
	 *
	 * @param input Index of the input.
	 * @return const operand_core& The core.
	 */
	const operand_core& get_input_core(std::size_t input) const noexcept;

	/**
	 * @brief Get the layout of the kept axes.
	 *
	 * @return const joint_layout& The inputs, then the outputs.
	 */
	const joint_layout& get_kept_layout() const noexcept;

	/**
	 * @brief Get the layout of the reduced axes.
	 *
	 * @return const joint_layout& The inputs.
	 */
	const joint_layout& get_reduced_layout() const noexcept;

	/**
	 * @brief Get the number of kept elements.
	 *
	 * @return std::size_t The count. At least one.
	 */
	std::size_t get_kept_count() const noexcept;

	/**
	 * @brief Get the number of reduced elements.
	 *
	 * @return std::size_t The count. At least one.
	 */
	std::size_t get_reduced_count() const noexcept;

	/**
	 * @brief Check whether an output has the layout of its accumulator.
	 *
	 * An output with the layout of its accumulator, over the same storage,
	 * is the accumulator updated in place.
	 *
	 * @param output Index of the output.
	 * @return bool True when the layouts are equal.
	 */
	bool output_matches_accumulator(std::size_t output) const noexcept;

private:
	std::size_t m_volume_extent;
	std::size_t m_slice_extent;
	std::vector<operand_core> m_output_cores;
	std::vector<operand_core> m_input_cores;
	joint_layout m_kept_layout;
	joint_layout m_reduced_layout;
	std::size_t m_kept_count;
	std::size_t m_reduced_count;
	std::vector<bool> m_output_matches_accumulator;
};

} // namespace cpu
} // namespace rexlib
