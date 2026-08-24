// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// The machinery a verb test uses to sweep every numerical type and to decide,
// from the operation's own typing rule, which of them it must accept and
// which it must reject. Shared by the fixtures of the operation families
// rather than restated by each, so that no test ever writes down a type list
// the declaration already holds.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <rexlib/functional/creation.hpp>

#include <rexlib/core/dispatch/rules/rule_operand_types.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/meta/type_tag.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>

#include <complex>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace rexlib
{
namespace test
{

/**
 * @brief Every numerical type an operand may have.
 *
 * Walked from the enumeration itself rather than listed, so a type added to
 * numerical_type is swept by every driver below without a test being
 * edited.
 *
 * @return std::vector<numerical_type> The types, in declaration order.
 */
inline std::vector<numerical_type> all_numerical_types()
{
	const auto count = static_cast<int>(numerical_type::count);

	std::vector<numerical_type> result;
	result.reserve(static_cast<std::size_t>(count));
	for (int i = 0; i < count; ++i)
	{
		result.push_back(static_cast<numerical_type>(i));
	}

	return result;
}

namespace detail
{

/**
 * @brief Build an element of a given type from a real and imaginary part.
 *
 * @tparam T The element type to build.
 */
template <typename T>
struct element_value_cast
{
	static T apply(double real, double) noexcept
	{
		return static_cast<T>(real);
	}
};

template <typename T>
struct element_value_cast<std::complex<T>>
{
	static std::complex<T> apply(double real, double imaginary) noexcept
	{
		return std::complex<T>(
			static_cast<T>(real),
			static_cast<T>(imaginary)
		);
	}
};

} // namespace detail

/**
 * @brief A test value expressed independently of the element type.
 *
 * Carries an imaginary part so that one description of a case serves both
 * real and complex element types, real ones taking the real part alone.
 * This is what lets a driver sweep a whole domain from a single case.
 */
class element_value
{
public:
	/**
	 * @brief Construct a value from its parts.
	 *
	 * @param real The real part, and the whole of a real element.
	 * @param imaginary The imaginary part, ignored by real elements.
	 */
	element_value(double real = 0.0, double imaginary = 0.0) noexcept
		: m_real(real)
		, m_imaginary(imaginary)
	{
	}

	/**
	 * @brief Get this value as a given element type.
	 *
	 * @tparam T The element type.
	 * @return T The value.
	 */
	template <typename T>
	T as() const noexcept
	{
		return detail::element_value_cast<T>::apply(m_real, m_imaginary);
	}

private:
	double m_real;
	double m_imaginary;
};

/**
 * @brief Widen a stored element into the type it is computed in.
 *
 * Mirrors the backend's load(), which widens float16_t because it has no
 * arithmetic of its own. A reference value is therefore computed in the
 * same type the kernel computes in, and rounded once, as the kernel rounds
 * it.
 *
 * @param value The stored element.
 * @return The element in its compute type.
 */
inline float32_t widen(float16_t value) noexcept
{
	return static_cast<float32_t>(value);
}

inline std::complex<float32_t> widen(
	const std::complex<float16_t> &value
) noexcept
{
	return std::complex<float32_t>(
		static_cast<float32_t>(value.real()),
		static_cast<float32_t>(value.imag())
	);
}

template <typename T>
const T& widen(const T &value) noexcept
{
	return value;
}

/**
 * @brief How closely a computed element must match its expected value.
 */
enum class comparison_mode
{
	exact,      ///< Bit for bit, once rounded to the stored type.
	approximate ///< Within a relative tolerance for the stored type.
};

namespace detail
{

/**
 * @brief Round a computed value back into its stored element type.
 *
 * Mirrors the backend's store(). Comparing stored values rather than
 * computed ones is what keeps the exact mode exact for float16_t.
 *
 * @tparam T The stored element type.
 */
template <typename T>
struct element_narrow
{
	template <typename V>
	static T apply(const V &value) noexcept
	{
		return static_cast<T>(value);
	}
};

template <typename T>
struct element_narrow<std::complex<T>>
{
	template <typename V>
	static std::complex<T> apply(const std::complex<V> &value) noexcept
	{
		return std::complex<T>(
			static_cast<T>(value.real()),
			static_cast<T>(value.imag())
		);
	}

	template <typename V>
	static std::complex<T> apply(const V &value) noexcept
	{
		return std::complex<T>(static_cast<T>(value));
	}
};

/**
 * @brief Compare one real component against its expected value.
 *
 * @param actual The computed component.
 * @param expected The expected component.
 * @param mode How closely they must match.
 * @param tolerance The relative tolerance used when approximating.
 */
inline void check_real_value(
	double actual,
	double expected,
	comparison_mode mode,
	double tolerance
)
{
	if (mode == comparison_mode::exact)
	{
		CHECK( actual == expected );
	}
	else
	{
		// The absolute alternative covers an expected value of zero,
		// against which no relative tolerance can be taken.
		CHECK_THAT(
			actual,
			Catch::Matchers::WithinRel(expected, tolerance) ||
			Catch::Matchers::WithinAbs(expected, tolerance)
		);
	}
}

/**
 * @brief Compare one stored element against its expected value.
 *
 * Integral, boolean and character results are exact whatever the mode:
 * there is no rounding for a tolerance to absorb.
 *
 * @tparam T The stored element type.
 */
template <typename T>
struct element_checker
{
	static void apply(const T &actual, const T &expected, comparison_mode)
	{
		CHECK( actual == expected );
	}
};

template <>
struct element_checker<float16_t>
{
	static void apply(
		float16_t actual,
		float16_t expected,
		comparison_mode mode
	)
	{
		// float16_t carries around three decimal digits.
		check_real_value(actual, expected, mode, 1e-2);
	}
};

template <>
struct element_checker<float32_t>
{
	static void apply(
		float32_t actual,
		float32_t expected,
		comparison_mode mode
	)
	{
		check_real_value(actual, expected, mode, 1e-5);
	}
};

template <>
struct element_checker<float64_t>
{
	static void apply(
		float64_t actual,
		float64_t expected,
		comparison_mode mode
	)
	{
		check_real_value(actual, expected, mode, 1e-12);
	}
};

template <typename T>
struct element_checker<std::complex<T>>
{
	static void apply(
		const std::complex<T> &actual,
		const std::complex<T> &expected,
		comparison_mode mode
	)
	{
		element_checker<T>::apply(actual.real(), expected.real(), mode);
		element_checker<T>::apply(actual.imag(), expected.imag(), mode);
	}
};

} // namespace detail

/**
 * @brief Reify the element type of every operand of an operation.
 *
 * Reads the operation's own typing rule, so the types a driver builds its
 * operands with cannot drift from the types the operation resolves. This is
 * what lets one driver serve where(), whose condition is boolean whatever
 * the swept type, and abs(), whose result is the real counterpart of it.
 *
 * @tparam Op The operation.
 * @tparam Pivot The free element type being swept.
 */
template <typename Op, typename Pivot>
struct operand_element_types
{
	using rule_type = typename Op::type_rule;

	static_assert(
		rule_type::pivot_count == 1,
		"The elementwise verb drivers sweep a single free element type. A "
		"rule declaring more than one pivot needs a driver that sweeps "
		"their combinations."
	);

	using outputs = rule_operand_types_t<
		typename rule_type::output_slot_list,
		type_list<Pivot>
	>;

	using inputs = rule_operand_types_t<
		typename rule_type::input_slot_list,
		type_list<Pivot>
	>;

	template <std::size_t I>
	using output_type = typename type_list_element<I, outputs>::type;

	template <std::size_t I>
	using input_type = typename type_list_element<I, inputs>::type;
};

/**
 * @brief Whether an operation admits an element type as its pivot.
 *
 * @tparam Op The operation.
 * @tparam Pivot The candidate element type.
 */
template <typename Op, typename Pivot>
struct operation_admits
	: rule_pivots_in_domain<
		typename Op::type_rule::pivot_list,
		type_list<Pivot>
	>
{
};

namespace detail
{

/**
 * @brief What a driver does with one of the types it sweeps.
 */
enum class case_kind
{
	skip,  ///< Outside the domain this case restricted itself to.
	run,   ///< Admitted by the operation: run the verb and compare.
	reject ///< Not admitted: the verb must reject it.
};

/**
 * @brief Decide what to do with one swept element type.
 *
 * A case that holds only over part of an operation's domain, such as the
 * sign a modulo takes, names the part as a domain rather than as a list of
 * types. Everything outside it is skipped; everything inside is still
 * checked against the operation's own domain, so restricting a case never
 * weakens what it asserts about the types it does cover.
 *
 * @tparam Op The operation.
 * @tparam Restriction The domain the case restricted itself to.
 * @tparam T The candidate element type.
 */
template <typename Op, typename Restriction, typename T>
struct select_case
	: std::integral_constant<
		case_kind,
		!Restriction::template contains_type<T>::value
			? case_kind::skip
			: (operation_admits<Op, T>::value
				? case_kind::run
				: case_kind::reject)
	>
{
};

template <case_kind K>
using case_tag = std::integral_constant<case_kind, K>;

using skip_case = case_tag<case_kind::skip>;
using run_case = case_tag<case_kind::run>;
using reject_case = case_tag<case_kind::reject>;

} // namespace detail

} // namespace test
} // namespace rexlib
