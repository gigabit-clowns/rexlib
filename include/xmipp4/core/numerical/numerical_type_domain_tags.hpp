// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "numerical_type.hpp"
#include "numerical_type_domain.hpp"
#include "numerical_type_traits.hpp"

#include "../platform/constexpr.hpp"

#include <type_traits>

namespace xmipp4
{

namespace detail
{

template <typename... Domains>
struct domain_union_value;

template <typename Domain>
struct domain_union_value<Domain>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value = Domain::value;
};

template <typename Domain, typename... Rest>
struct domain_union_value<Domain, Rest...>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value =
		Domain::value | domain_union_value<Rest...>::value;
};

template <typename... Domains>
struct domain_intersection_value;

template <typename Domain>
struct domain_intersection_value<Domain>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value = Domain::value;
};

template <typename Domain, typename... Rest>
struct domain_intersection_value<Domain, Rest...>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value =
		Domain::value & domain_intersection_value<Rest...>::value;
};

} // namespace detail

/**
 * @brief Base giving a named domain both of its halves.
 *
 * A domain has to answer the same question in two places: the rule engine
 * asks it about a numerical_type value, and a backend asks it about a
 * native C++ type in order to decide whether to instantiate a kernel at
 * all. Both come from the single constexpr mask @p Derived declares, so
 * they cannot disagree.
 *
 * @tparam Derived The named domain, exposing a constexpr `value`.
 */
template <typename Derived>
struct named_domain
{
	/**
	 * @brief Get the domain.
	 *
	 * @return const numerical_type_domain& The domain.
	 */
	static const numerical_type_domain& get() noexcept
	{
		return Derived::value;
	}

	/**
	 * @brief Check at compile time whether a static type belongs.
	 *
	 * @tparam T The static type.
	 */
	template <typename T>
	struct contains_type
		: std::integral_constant<
			bool,
			Derived::value.contains(numerical_type_of<T>::value)
		>
	{
	};
};

/**
 * @brief Named domain holding every type of a given category.
 *
 * Domains are named by tag types rather than by values, so that they can be
 * used as template arguments.
 *
 * @tparam Category The category of the types belonging to the domain.
 */
template <numerical_type_category Category>
struct category_type_domain
	: named_domain<category_type_domain<Category>>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value =
		make_numerical_type_domain(Category);
};

using boolean_type_domain =
	category_type_domain<numerical_type_category::boolean>;
using character_type_domain =
	category_type_domain<numerical_type_category::character>;
using signed_integer_type_domain =
	category_type_domain<numerical_type_category::signed_integer>;
using unsigned_integer_type_domain =
	category_type_domain<numerical_type_category::unsigned_integer>;
using floating_point_type_domain =
	category_type_domain<numerical_type_category::floating_point>;
using complex_type_domain =
	category_type_domain<numerical_type_category::complex>;

/**
 * @brief Named domain holding the union of other named domains.
 *
 * @tparam Domains The domain tags to be united. At least one is required.
 */
template <typename... Domains>
struct domain_union
	: named_domain<domain_union<Domains...>>
{
	static_assert(
		sizeof...(Domains) > 0,
		"domain_union requires at least one domain. An empty union would "
		"silently admit no type at all."
	);

	static REX_CONST_CONSTEXPR numerical_type_domain value =
		detail::domain_union_value<Domains...>::value;
};

/**
 * @brief Named domain holding the intersection of other named domains.
 *
 * This is how a backend narrows the domain of an operation without
 * restating it.
 *
 * @tparam Domains The domain tags to be intersected. At least one is
 * required.
 */
template <typename... Domains>
struct domain_intersection
	: named_domain<domain_intersection<Domains...>>
{
	static_assert(
		sizeof...(Domains) > 0,
		"domain_intersection requires at least one domain. An empty "
		"intersection would silently admit every type."
	);

	static REX_CONST_CONSTEXPR numerical_type_domain value =
		detail::domain_intersection_value<Domains...>::value;
};

/**
 * @brief Named domain holding what one domain has and another lacks.
 *
 * States an exclusion directly, which reads better than the intersection
 * with a complement when an operation admits everything but a handful of
 * types.
 *
 * @tparam Domain The domain to take types from.
 * @tparam Excluded The domain whose types are removed.
 */
template <typename Domain, typename Excluded>
struct domain_difference
	: named_domain<domain_difference<Domain, Excluded>>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value =
		Domain::value & ~Excluded::value;
};

/**
 * @brief Named domain holding every representable type.
 */
struct any_type_domain
	: named_domain<any_type_domain>
{
	static REX_CONST_CONSTEXPR numerical_type_domain value =
		numerical_type_domain::all();
};

/// Signed and unsigned integers.
using integral_type_domain = domain_union<
	signed_integer_type_domain,
	unsigned_integer_type_domain
>;

/// Floating point and complex types, i.e. those with a fractional part.
using inexact_type_domain = domain_union<
	floating_point_type_domain,
	complex_type_domain
>;

/// Integers and floating point types, i.e. arithmetic types on the real line.
using real_arithmetic_type_domain = domain_union<
	integral_type_domain,
	floating_point_type_domain
>;

/// Every type arithmetic operations are defined on.
using arithmetic_type_domain = domain_union<
	integral_type_domain,
	inexact_type_domain
>;

/// Arithmetic types that can represent a negative value.
using signed_arithmetic_type_domain = domain_union<
	signed_integer_type_domain,
	inexact_type_domain
>;

} // namespace xmipp4
