// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "extent_domain.hpp"
#include "operand_axis_slot.hpp"

#include <cstddef>

namespace rexlib
{

/**
 * @brief Type erased form of an operand_axis_slot.
 *
 * @see operand_axis_slot
 */
class axis_slot_descriptor
{
public:
	/**
	 * @brief Function applying the transform of a slot.
	 */
	using transform_function = std::size_t (*)(std::size_t);

	/**
	 * @brief Construct an axis slot descriptor.
	 *
	 * @param kind How the axis gets its extent.
	 * @param index The index of the extent variable, or the fixed extent,
	 * as @p kind requires.
	 * @param apply The transform applied to the extent variable. Must not be
	 * null.
	 * @param relation Fragment naming the transform, for diagnostics. Must
	 * not be null and must have static storage duration.
	 * @param domain The extents the axes of a run may take.
	 */
	axis_slot_descriptor(
		axis_slot_kind kind,
		std::size_t index,
		transform_function apply,
		const char *relation,
		const extent_domain &domain
	) noexcept;

	/**
	 * @brief Get how the axis gets its extent.
	 *
	 * @return axis_slot_kind The kind.
	 */
	axis_slot_kind get_kind() const noexcept;

	/**
	 * @brief Get the index of the extent variable, or the fixed extent.
	 *
	 * @return std::size_t The index or the extent, as the kind requires.
	 */
	std::size_t get_index() const noexcept;

	/**
	 * @brief Apply the transform to the extent of the variable.
	 *
	 * @param extent The extent of the variable.
	 * @return std::size_t The extent of the axis.
	 */
	std::size_t apply(std::size_t extent) const noexcept;

	/**
	 * @brief Get the fragment naming the transform.
	 *
	 * @return const char* The fragment.
	 */
	const char* get_relation() const noexcept;

	/**
	 * @brief Get the extents the axes of a run may take.
	 *
	 * @return const extent_domain& The domain.
	 */
	const extent_domain& get_domain() const noexcept;

private:
	axis_slot_kind m_kind;
	std::size_t m_index;
	transform_function m_apply;
	const char *m_relation;
	extent_domain m_domain;
};

} // namespace rexlib

#include "axis_slot_descriptor.inl"
