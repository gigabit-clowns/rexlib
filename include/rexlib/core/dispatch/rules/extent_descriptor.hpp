// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "extent_domain.hpp"
#include "operand_extent.hpp"

namespace rexlib
{

/**
 * @brief Type erased form of an operand_extent.
 *
 * @see operand_extent
 */
class extent_descriptor
{
public:
	/**
	 * @brief Construct an extent descriptor.
	 *
	 * @param source Where the value of the variable comes from.
	 * @param domain The extents the variable may take.
	 */
	extent_descriptor(
		extent_source source,
		const extent_domain &domain
	) noexcept;

	/**
	 * @brief Get where the value of the variable comes from.
	 *
	 * @return extent_source The source.
	 */
	extent_source get_source() const noexcept;

	/**
	 * @brief Get the extents the variable may take.
	 *
	 * @return const extent_domain& The domain.
	 */
	const extent_domain& get_domain() const noexcept;

private:
	extent_source m_source;
	extent_domain m_domain;
};

} // namespace rexlib

#include "extent_descriptor.inl"
