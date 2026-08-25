// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <limits>
#include <cstdint>
#include <type_traits>

#include "../platform/attributes.hpp"
#include "../platform/dynamic_shared_object.h"

namespace rexlib
{

/**
 * @brief 16 bit floating point number representation.
 *
 * As there is no standardized half-width float type in C/C++ use a
 * placeholder.
 */
class REXLIB_API float16_t
{
public:
	float16_t() noexcept;
	float16_t(float value) noexcept;
	float16_t(const float16_t &other) = default;
	float16_t(float16_t &&other) = default;
	~float16_t() = default;

	float16_t& operator=(const float16_t &other) = default;
	float16_t& operator=(float16_t &&other) = default;

	operator float() const noexcept;

	/**
	 * @brief Obtain the underlying bit pattern.
	 *
	 * @return std::uint16_t Unsigned integer with the bits.
	 */
	std::uint16_t get_bits() const noexcept;

private:
	std::uint16_t m_bits;
};
static_assert(sizeof(float16_t) == 2, "float16_t should be 2 bytes long");

/**
 * @brief 32 bit floating point number representation.
 */
using float32_t = float;
static_assert(sizeof(float32_t) == 4, "float32_t should be 4 bytes long");

/**
 * @brief 64 bit floating point number representation.
 */
using float64_t = double;
static_assert(sizeof(float64_t) == 8, "float64_t should be 8 bytes long");

/**
 * @brief Determine if a type is a floating point type.
 *
 * Behaves like @c std::is_floating_point, but also considers
 * @ref float16_t to be a floating point type, even though it does not
 * provide native arithmetic support.
 *
 * @tparam T The type to be checked.
 */
template <typename T>
struct is_floating_point : std::integral_constant<
	bool,
	std::is_floating_point<T>::value || std::is_same<T, float16_t>::value
> {};

} // namespace rexlib
