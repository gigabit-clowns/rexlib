// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <vector>

namespace rexlib
{

/**
 * @brief What a shape rule resolves to against a set of inputs.
 *
 * Holds everything the shape of an output is assembled from: the batch
 * axes the inputs broadcast to, the extent of every extent variable, and
 * the extents of the axes of the run.
 */
class shape_rule_resolution
{
public:
	using extent_list = std::vector<std::size_t>;

	/**
	 * @brief Construct a resolution.
	 *
	 * @param batch The extents of the batch axes, in the order they are laid
	 * out, the named axes left out.
	 * @param extents The extent of every extent variable, in declaration
	 * order.
	 * @param run_extents The extents of the axes of the run, in the order
	 * they are laid out. Empty when the rule declares no run.
	 */
	shape_rule_resolution(
		extent_list batch,
		extent_list extents,
		extent_list run_extents
	) noexcept;

	/**
	 * @brief Get the extents of the batch axes.
	 *
	 * @return const extent_list& The extents, in the order they are laid
	 * out.
	 */
	const extent_list& get_batch() const noexcept;

	/**
	 * @brief Get the extent of an extent variable.
	 *
	 * @param index Index of the variable. Must be less than the number of
	 * variables of the rule.
	 * @return std::size_t The extent.
	 */
	std::size_t get_extent(std::size_t index) const noexcept;

	/**
	 * @brief Get the extents of the axes of the run.
	 *
	 * @return const extent_list& The extents, in the order they are laid
	 * out.
	 */
	const extent_list& get_run_extents() const noexcept;

private:
	extent_list m_batch;
	extent_list m_extents;
	extent_list m_run_extents;
};

} // namespace rexlib
