// SPDX-License-Identifier: GPL-3.0-only

#include "numerical_cast.hpp"

#include "fixed_width_float.hpp"

namespace rexlib
{

namespace detail
{

/**
 * @brief Trait detecting @c std::complex specializations.
 */
template <typename T>
struct is_complex : std::false_type {};

template <typename T>
struct is_complex<std::complex<T>> : std::true_type {};

/**
 * @brief Implementation of @ref numerical_cast selected by partial
 * specialization.
 *
 * The primary template handles every pair that does not involve
 * @ref float16_t nor @c std::complex, deferring to a plain @c static_cast.
 */
template <typename T, typename Q, typename = void>
struct numerical_cast_helper
{
	static T cast(const Q &src) noexcept
	{
		return static_cast<T>(src);
	}
};

/**
 * @brief Conversion towards a scalar @ref float16_t.
 *
 * Makes the source-to-@c float step explicit before building the
 * @ref float16_t.
 */
template <typename Q>
struct numerical_cast_helper<
	float16_t, Q,
	typename std::enable_if<!std::is_same<Q, float16_t>::value>::type
>
{
	static float16_t cast(const Q &src) noexcept
	{
		return float16_t(static_cast<float>(src));
	}
};

/**
 * @brief Conversion away from a scalar @ref float16_t.
 *
 * Makes the @c float-to-destination step explicit after reading the
 * @ref float16_t.
 */
template <typename T>
struct numerical_cast_helper<
	T, float16_t,
	typename std::enable_if<
		!std::is_same<T, float16_t>::value && !is_complex<T>::value
	>::type
>
{
	static T cast(const float16_t &src) noexcept
	{
		return static_cast<T>(static_cast<float>(src));
	}
};

/**
 * @brief Conversion between two @c std::complex values.
 *
 * Applied element-wise so that @ref float16_t components go through the scalar
 * specializations above.
 */
template <typename T, typename Q>
struct numerical_cast_helper<std::complex<T>, std::complex<Q>>
{
	static std::complex<T> cast(const std::complex<Q> &src) noexcept
	{
		return std::complex<T>(
			numerical_cast<T>(src.real()),
			numerical_cast<T>(src.imag())
		);
	}
};

/**
 * @brief Conversion from a scalar into a @c std::complex value.
 *
 * The scalar becomes the real part; the imaginary part is left at zero. The
 * real part goes through the scalar conversion so that a @ref float16_t
 * component is handled explicitly.
 */
template <typename T, typename Q>
struct numerical_cast_helper<
	std::complex<T>, Q,
	typename std::enable_if<!is_complex<Q>::value>::type
>
{
	static std::complex<T> cast(const Q &src) noexcept
	{
		return std::complex<T>(numerical_cast<T>(src));
	}
};

} // namespace detail

template <typename T, typename Q>
REXLIB_NODISCARD
T numerical_cast(const Q &src) noexcept
{
	return detail::numerical_cast_helper<T, Q>::cast(src);
}

} // namespace rexlib
