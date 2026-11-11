// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/loops/indices/element_index_tags.hpp>
#include <backends/cpu/loops/layouts/named_core_frame.hpp>
#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/operation/shape_rule/core_dimensions_descriptor.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <type_traits>
#include <vector>

namespace rexlib
{

class operand_descriptor;

namespace cpu
{

/**
 * @brief The index a batch loop over a batched_map_plan is handed for an
 * index of the map: the batch part of the counter for a linear one, and
 * none otherwise.
 *
 * @tparam MapIndexing The index of the map.
 */
template <typename MapIndexing>
using batch_indexing_of = typename std::conditional<
	std::is_same<MapIndexing, linear_index_tag>::value,
	linear_index_tag,
	no_index_tag
>::type;

/**
 * @brief The iteration of a map over the core of every batch element.
 *
 * The frame is that of the outputs: its core axes are the core of the
 * outputs, and the batch axes are the rest. The batch layout visits every
 * batch element once, and for each of them the map layout walks the core.
 * The operand offsets belong to the batch layout alone: the map layout
 * starts its operands at zero and displaces the pointers the batch layout
 * has placed.
 *
 * An input is either mapped, walked by the map layout as the outputs are,
 * or read once per batch element: it then has a core of its own, its
 * trailing axes, possibly none, which the plan keeps as an operand_core and
 * which takes part in the batch layout only.
 *
 * The map may be handed an index. A linear one is the counter: the
 * row-major position of an element among the elements of the result that
 * share the values of every input read once per batch element. It is split
 * in two operands, one in the batch layout and one in the map layout, whose
 * offsets add up to it. A multidimensional one is the position of an
 * element within the core, an operand of the map layout alone.
 */
class batched_map_plan
{
public:
	/**
	 * @brief Plan a map over the core of the outputs.
	 *
	 * @tparam MapIndexing The index of the map: no_index_tag,
	 * linear_index_tag or multidimensional_index_tag.
	 * @param frame The axes of the outputs, split into their core and their
	 * batch axes.
	 * @param output_descriptors Descriptors of the outputs, each of the
	 * extents of the frame.
	 * @param input_descriptors Descriptors of the inputs.
	 * @param input_cores The core of every input, as the shape rule declares
	 * it.
	 * @param mapped_inputs The inputs the map walks, ascending. An empty
	 * core marks one broadcast against the batch axes. Every other input is
	 * read once per batch element, as many trailing axes of it as its core
	 * declares, at most three, being its core.
	 * @param map_indexing Selects the index of the map.
	 * @throws std::invalid_argument When there is no output, when the cores
	 * do not match the inputs, when an input read once per batch element has
	 * a run of axes or more than three, or when an operand does not broadcast
	 * to the frame.
	 */
	template <typename MapIndexing>
	batched_map_plan(
		named_core_frame frame,
		span<const operand_descriptor> output_descriptors,
		span<const operand_descriptor> input_descriptors,
		span<const core_dimensions_descriptor> input_cores,
		span<const std::size_t> mapped_inputs,
		MapIndexing map_indexing
	);

	batched_map_plan(const batched_map_plan &other) = delete;
	batched_map_plan(batched_map_plan &&other) noexcept = default;
	~batched_map_plan() = default;

	batched_map_plan& operator=(const batched_map_plan &other) = delete;
	batched_map_plan& operator=(batched_map_plan &&other) noexcept = default;

	/**
	 * @brief Get the core axes and the batch axes of the result.
	 *
	 * @return const named_core_frame& The frame.
	 */
	const named_core_frame& get_frame() const noexcept;

	/**
	 * @brief Get the core of an input read once per batch element.
	 *
	 * @param input The index of the input.
	 * @return const operand_core& Its core, of rank 0 for an input without
	 * core axes or one the map walks.
	 */
	const operand_core& get_input_core(std::size_t input) const noexcept;

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
	 * @brief Get the layout the map walks the core axes through.
	 *
	 * Its operands are the outputs, in order, followed by the mapped
	 * inputs, in order, followed by the core part of the counter or the
	 * index operands of the core when there are any.
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
	joint_layout m_map_layout;
	std::vector<operand_core> m_input_cores;
	std::size_t m_batch_count;
	std::size_t m_core_count;
};

} // namespace cpu
} // namespace rexlib
