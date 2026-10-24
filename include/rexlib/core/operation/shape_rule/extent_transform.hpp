// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>

namespace rexlib
{

/**
 * @brief Type erased form of an extent transform.
 *
 * Holds the mapping, the fragment naming it in diagnostics, and whether it
 * is the identity.
 *
 * @see same_extent_transform
 */
class extent_transform
{
public:
	/**
	 * @brief The mapping of an extent.
	 */
	using apply_function = std::size_t (*)(std::size_t);

	/**
	 * @brief Construct an extent transform.
	 *
	 * @param apply The mapping. Must not be null.
	 * @param relation Fragment naming the mapping, for diagnostics. Must not
	 * be null and must outlive the transform.
	 * @param identity Whether the mapping is the identity.
	 */
	extent_transform(
		apply_function apply,
		const char *relation,
		bool identity
	) noexcept;

	/**
	 * @brief Apply the transform to an extent.
	 *
	 * @param extent The extent of the variable.
	 * @return std::size_t The extent of the axis.
	 */
	std::size_t apply(std::size_t extent) const noexcept;

	/**
	 * @brief Get the fragment naming the mapping.
	 *
	 * @return const char* The fragment.
	 */
	const char* get_relation() const noexcept;

	/**
	 * @brief Check whether the transform is the identity.
	 *
	 * @return true The transform maps every extent onto itself.
	 * @return false It does not.
	 */
	bool is_identity() const noexcept;

private:
	apply_function m_apply;
	const char *m_relation;
	bool m_identity;
};

/**
 * @brief Erase an extent transform tag.
 *
 * @tparam Transform An extent transform, such as same_extent_transform.
 * @return extent_transform The erased transform.
 */
template <typename Transform>
extent_transform make_extent_transform() noexcept;

} // namespace rexlib

#include "extent_transform.inl"
