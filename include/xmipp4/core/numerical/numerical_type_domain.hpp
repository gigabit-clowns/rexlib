// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "numerical_type.hpp"

#include "../platform/constexpr.hpp"
#include "../platform/dynamic_shared_object.h"

#include <cstdint>
#include <initializer_list>
#include <ostream>

namespace xmipp4
{

/**
 * @brief Set of numerical_type-s.
 *
 * Represents which numerical types are admissible in a given context, such
 * as the operand types an operation accepts or the subset of those a
 * backend is able to execute. Domains compose with the usual set
 * operations, which lets a backend narrow an operation's domain without
 * restating it.
 *
 * The set is stored as a bit mask indexed by the numerical_type
 * enumerator, so it is a trivially copyable value type.
 *
 * @see numerical_type
 */
class numerical_type_domain
{
public:
	/**
	 * @brief Underlying bit mask representation.
	 */
	using mask_type = std::uint32_t;

	/**
	 * @brief Construct an empty domain.
	 */
	REX_CONSTEXPR
	numerical_type_domain() noexcept;

	/**
	 * @brief Construct a domain from its bit mask representation.
	 *
	 * @param mask The bit mask, indexed by numerical_type enumerator.
	 */
	REX_CONSTEXPR
	explicit numerical_type_domain(mask_type mask) noexcept;

	/**
	 * @brief Construct a domain from an explicit list of types.
	 *
	 * Types with no bit representation, such as numerical_type::unknown,
	 * are ignored.
	 *
	 * @param types The types belonging to the domain.
	 */
	REX_CONSTEXPR_CPP14
	numerical_type_domain(
		std::initializer_list<numerical_type> types
	) noexcept;

	numerical_type_domain(const numerical_type_domain &other) = default;
	numerical_type_domain(numerical_type_domain &&other) noexcept = default;
	~numerical_type_domain() = default;

	numerical_type_domain&
	operator=(const numerical_type_domain &other) = default;
	numerical_type_domain&
	operator=(numerical_type_domain &&other) noexcept = default;

	/**
	 * @brief Check whether a type belongs to this domain.
	 *
	 * @param type The type to be tested.
	 * @return true The type belongs to the domain.
	 * @return false The type does not belong to the domain, or has no bit
	 * representation.
	 */
	REX_CONSTEXPR
	bool contains(numerical_type type) const noexcept;

	/**
	 * @brief Check whether the domain admits no type at all.
	 *
	 * @return true The domain is empty.
	 * @return false The domain admits at least one type.
	 */
	REX_CONSTEXPR
	bool empty() const noexcept;

	/**
	 * @brief Get the bit mask representation of the domain.
	 *
	 * @return mask_type The bit mask.
	 */
	REX_CONSTEXPR
	mask_type get_mask() const noexcept;

	/**
	 * @brief Create the domain containing every representable type.
	 *
	 * @return numerical_type_domain The full domain.
	 */
	static REX_CONSTEXPR
	numerical_type_domain all() noexcept;

	/**
	 * @brief Create the domain containing no type.
	 *
	 * @return numerical_type_domain The empty domain.
	 */
	static REX_CONSTEXPR
	numerical_type_domain none() noexcept;

	friend REX_CONSTEXPR bool operator==(
		const numerical_type_domain &lhs,
		const numerical_type_domain &rhs
	) noexcept
	{
		return lhs.get_mask() == rhs.get_mask();
	}

	friend REX_CONSTEXPR bool operator!=(
		const numerical_type_domain &lhs,
		const numerical_type_domain &rhs
	) noexcept
	{
		return !(lhs == rhs);
	}

	/**
	 * @brief Compute the intersection of two domains.
	 */
	friend REX_CONSTEXPR numerical_type_domain operator&(
		const numerical_type_domain &lhs,
		const numerical_type_domain &rhs
	) noexcept
	{
		return numerical_type_domain(lhs.get_mask() & rhs.get_mask());
	}

	/**
	 * @brief Compute the union of two domains.
	 */
	friend REX_CONSTEXPR numerical_type_domain operator|(
		const numerical_type_domain &lhs,
		const numerical_type_domain &rhs
	) noexcept
	{
		return numerical_type_domain(lhs.get_mask() | rhs.get_mask());
	}

	/**
	 * @brief Write a human readable representation of a domain.
	 *
	 * The types are listed in enumerator order, separated by commas.
	 *
	 * @param os The stream where the domain is written.
	 * @param domain The domain to be written.
	 * @return std::ostream& The stream.
	 */
	friend REXLIB_API std::ostream& operator<<(
		std::ostream &os,
		const numerical_type_domain &domain
	);

private:
	mask_type m_mask;
};

/**
 * @brief Check whether a type has a bit representation within a domain.
 *
 * Only the enumerators in [numerical_type::boolean, numerical_type::count)
 * are representable. In particular numerical_type::unknown is not.
 *
 * @param type The type to be tested.
 * @return true The type is representable.
 * @return false The type is not representable.
 */
REX_CONSTEXPR
bool is_domain_representable(numerical_type type) noexcept;

/**
 * @brief Compute the complement of a domain.
 *
 * The complement is taken with respect to the set of all representable
 * types, so no unrepresentable bit is ever set.
 */
REX_CONSTEXPR
numerical_type_domain operator~(const numerical_type_domain &domain) noexcept;

/**
 * @brief Create the domain holding every type of a given category.
 *
 * @param category The category of the types belonging to the domain.
 * @return numerical_type_domain The domain.
 *
 * @see numerical_type_category
 */
REX_CONSTEXPR_CPP14
numerical_type_domain make_numerical_type_domain(
	numerical_type_category category
) noexcept;



} // namespace xmipp4

#include "numerical_type_domain.inl"
