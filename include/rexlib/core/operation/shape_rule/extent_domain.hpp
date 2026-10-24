// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{

/**
 * @brief Set of extents an axis may take.
 *
 * Type erased form of an extent domain tag, such as
 * positive_extent_domain, so that a rule lowered to tables refers to its
 * domains without their types.
 */
class extent_domain
{
public:
	/**
	 * @brief Membership test of a domain.
	 */
	using contains_function_type = bool (*)(std::size_t);

	/**
	 * @brief Construct a domain.
	 *
	 * @param contains The membership test. Must not be null.
	 * @param description What the domain admits, for diagnostics, such as
	 * "at least one". Must not be null and must have static storage
	 * duration.
	 */
	extent_domain(
		contains_function_type contains,
		const char *description
	) noexcept;

	/**
	 * @brief Check whether the domain admits an extent.
	 *
	 * @param extent The extent.
	 * @return true The extent belongs to the domain.
	 * @return false The extent does not belong to the domain.
	 */
	bool contains(std::size_t extent) const noexcept;

	/**
	 * @brief Get what the domain admits.
	 *
	 * @return const char* The description.
	 */
	const char* get_description() const noexcept;

private:
	contains_function_type m_contains;
	const char *m_description;
};

/**
 * @brief Build the domain an extent domain tag names.
 *
 * @tparam Domain The tag. It must provide `static bool contains(std::size_t)`
 * and `static const char* get_description()`.
 * @return extent_domain The domain.
 */
template <typename Domain>
extent_domain make_extent_domain() noexcept;

} // namespace rexlib

#include "extent_domain.inl"
