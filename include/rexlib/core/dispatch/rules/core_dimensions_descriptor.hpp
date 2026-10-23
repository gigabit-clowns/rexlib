// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "axis_slot_descriptor.hpp"

#include <rexlib/core/span.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Type erased form of a core_dimensions.
 *
 * @see core_dimensions
 */
class core_dimensions_descriptor
{
public:
	/**
	 * @brief Construct a descriptor of the core axes of an operand.
	 *
	 * @param slots The slots of the core axes, in the order the axes are
	 * laid out. They must outlive the descriptor and hold at most one run.
	 */
	explicit core_dimensions_descriptor(
		span<const axis_slot_descriptor> slots
	) noexcept;

	/**
	 * @brief Get the slots of the core axes.
	 *
	 * @return span<const axis_slot_descriptor> The slots.
	 */
	span<const axis_slot_descriptor> get_slots() const noexcept;

	/**
	 * @brief Check whether one of the slots is a run.
	 *
	 * @return true A slot is a run.
	 * @return false No slot is a run.
	 */
	bool has_run() const noexcept;

	/**
	 * @brief Get the position of the run among the slots.
	 *
	 * @return std::size_t The position, or the number of slots when there
	 * is no run.
	 */
	std::size_t get_run_position() const noexcept;

	/**
	 * @brief Get the number of slots that are not a run.
	 *
	 * @return std::size_t The number of slots standing for one axis each.
	 */
	std::size_t get_axis_count() const noexcept;

private:
	span<const axis_slot_descriptor> m_slots;
	std::size_t m_run_position;
};

} // namespace rexlib

#include "core_dimensions_descriptor.inl"
