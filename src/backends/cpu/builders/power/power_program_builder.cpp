// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/power/power_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/builders/default_kernel_factory.hpp>
#include <backends/cpu/load_store.hpp>

#include <cmath>
#include <complex>
#include <type_traits>

namespace rex
{
namespace cpu
{

namespace
{

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
is_negative(T) noexcept
{
	return false;
}

template <typename T>
typename std::enable_if<!std::is_unsigned<T>::value, bool>::type
is_negative(T value) noexcept
{
	return value < T(0);
}

/**
 * @brief Raise an integer to an integer power.
 *
 * Squaring rather than going through std::pow, whose double result cannot
 * represent every 64 bit integer and would round before truncating back.
 *
 * A negative exponent has no representable result: the value lies strictly
 * between -1 and 1 for every base but those two, so it truncates to zero,
 * as dividing one by a power would. NumPy raises instead, which a kernel
 * cannot do per element without abandoning the loop.
 *
 * @tparam T The integer element type.
 * @param base The base.
 * @param exponent The exponent.
 * @return T The power.
 */
template <typename T>
T integer_power(T base, T exponent) noexcept
{
	if (is_negative(exponent))
	{
		if (base == T(1))
		{
			return T(1);
		}
		if (base == static_cast<T>(-1))
		{
			// Alternates with the parity of the exponent, whose sign does
			// not affect it.
			return (exponent % T(2) == T(0)) ? T(1) : static_cast<T>(-1);
		}

		return T(0);
	}

	T result(1);
	while (exponent > T(0))
	{
		if (exponent % T(2) != T(0))
		{
			result = static_cast<T>(result * base);
		}

		base = static_cast<T>(base * base);
		exponent = static_cast<T>(exponent / T(2));
	}

	return result;
}

struct power_kernel
{
	template <typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type
	operator()(T *result, const T *x, const T *y) const noexcept
	{
		store(result, integer_power(load(x), load(y)));
	}

	// Also serves float16_t, which load() widens into a type std::pow
	// accepts, and the complex types.
	template <typename T>
	typename std::enable_if<!std::is_integral<T>::value, void>::type
	operator()(T *result, const T *x, const T *y) const noexcept
	{
		using std::pow;
		store(result, pow(load(x), load(y)));
	}
};

} // anonymous namespace

REX_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	power,
	ops::power_operation,
	default_kernel_factory<power_kernel>
);

} // namespace cpu
} // namespace rex
