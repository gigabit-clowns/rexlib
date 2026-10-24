// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/shape_rule/extent_domain.hpp>
#include <rexlib/core/operation/shape_rule/extent_pivot_source.hpp>

namespace rexlib
{

/**
 * @brief Type erased form of an extent_pivot.
 *
 * @see extent_pivot
 */
class extent_pivot_descriptor
{
public:
	/**
	 * @brief Construct an extent descriptor.
	 *
	 * @param source Where the value of the variable comes from.
	 * @param domain The extents the variable may take.
	 */
	extent_pivot_descriptor(
		extent_pivot_source source,
		const extent_domain &domain
	) noexcept;

	/**
	 * @brief Get where the value of the variable comes from.
	 *
	 * @return extent_pivot_source The source.
	 */
	extent_pivot_source get_source() const noexcept;

	/**
	 * @brief Get the extents the variable may take.
	 *
	 * @return const extent_domain& The domain.
	 */
	const extent_domain& get_domain() const noexcept;

private:
	extent_pivot_source m_source;
	extent_domain m_domain;
};

} // namespace rexlib

#include "extent_pivot_descriptor.inl"
