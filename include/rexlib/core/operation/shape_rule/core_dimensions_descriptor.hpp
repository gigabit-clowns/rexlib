// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/extent_slot_descriptor.hpp>

#include <rexlib/core/span.hpp>

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
	 * @brief Construct a core dimensions descriptor.
	 *
	 * @param slots The slot of every core axis after the run. The table must
	 * outlive the descriptor.
	 * @param has_run Whether a run leads the core axes.
	 * @param run_domain The extents every axis of the run may take.
	 */
	core_dimensions_descriptor(
		span<const extent_slot_descriptor> slots,
		bool has_run,
		const extent_domain &run_domain
	) noexcept;

	/**
	 * @brief Get the slot of every core axis after the run.
	 *
	 * @return span<const extent_slot_descriptor> One entry per axis.
	 */
	span<const extent_slot_descriptor> get_slots() const noexcept;

	/**
	 * @brief Check whether a run leads the core axes.
	 *
	 * @return true A run leads them.
	 * @return false There is no run.
	 */
	bool has_run() const noexcept;

	/**
	 * @brief Check whether the core has no axes: neither a run nor a slot.
	 *
	 * @return true The core has no axes.
	 * @return false The core has a run or a slot.
	 */
	bool is_empty() const noexcept;

	/**
	 * @brief Get the extents every axis of the run may take.
	 *
	 * @return const extent_domain& The domain.
	 */
	const extent_domain& get_run_domain() const noexcept;

private:
	span<const extent_slot_descriptor> m_slots;
	bool m_has_run;
	extent_domain m_run_domain;
};

} // namespace rexlib

#include "core_dimensions_descriptor.inl"
