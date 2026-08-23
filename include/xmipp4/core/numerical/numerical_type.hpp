// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../platform/dynamic_shared_object.h"

#include <ostream>

namespace xmipp4
{

/**
 * @brief Representation of all considered numerical types for
 * computations.
 * 
 */
enum class numerical_type
{
	unknown = -1,

	boolean,

	char8,

	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	int64,
	uint64,

	float16,
	float32,
	float64,

	complex_float16,
	complex_float32,
	complex_float64,

	// Add here

	count
};

/**
 * @brief Broad categorical classification of `numerical_type`.
 * 
 * @see numerical_type
 */
enum class numerical_type_category
{
	unknown = -1,

	boolean,
	character,
	signed_integer,
	unsigned_integer,
	floating_point,
	complex,

	// Add here

	count
};

/**
 * @brief Get the size of a numerical type.
 * 
 * @param type The type for which the size should be obtained.
 * @return std::size_t The size.
 */
REXLIB_API
std::size_t get_size(numerical_type type) noexcept;

/**
 * @brief Create a complex version, if exists.
 * 
 * Create the complex version of a numerical type, if this exists.
 * Otherwise unknown is returned. This includes an already complex type.
 * 
 * @param type 
 * @return numerical_type.
 */
REXLIB_API
numerical_type make_complex(numerical_type type) noexcept;

/**
 * @brief Get the real equivalent of a numerical type.
 *
 * For complex types, returns the corresponding floating-point type
 * (e.g. complex_float32 -> float32). For other valid types it returns the 
 * type itself.
 *
 * @param type A potentially complex numerical type.
 * @return numerical_type The equivalent real type. numerical_type::unknown
 * if error.
 */
REXLIB_API
numerical_type make_real(numerical_type type) noexcept;

/**
 * @brief Get the inexact equivalent of a numerical type.
 *
 * Types that can only represent whole values, which is to say booleans and
 * integers, map onto float64, the widest type able to hold their result.
 * Floating point and complex types already have a fractional part and are
 * returned unchanged.
 *
 * Models the result type of operations whose value is not representable in
 * the operand's own type, such as a mean. Characters are not admitted: they
 * are not a numeric quantity to average.
 *
 * @param type The type to be made inexact.
 * @return numerical_type The equivalent inexact type.
 * numerical_type::unknown if error.
 */
REXLIB_API
numerical_type make_inexact(numerical_type type) noexcept;

/**
 * @brief Get the common type of two numerical types.
 * 
 * The type promotion mechanism is heavily influenced by JAX's approach:
 * https://docs.jax.dev/en/latest/jep/9407-type-promotion.html
 * 
 * However, unlike JAX, it allows type promotion from uint64 to int64.
 * 
 * @param type1 First numerical type.
 * @param type2 Second numerical type.
 * @return numerical_type The common type.
 */
REXLIB_API
numerical_type 
promote_types(numerical_type type1, numerical_type type2) noexcept;

/**
 * @brief Get the category of the provided numerical type.
 * 
 * @param type The type from which the category is inferred.
 * @return numerical_type_category The category of the input type or
 * `numerical_type_category::unknown` if error.
 *
 * @note Usable in a constant expression, so that the same definition
 * serves both a runtime classification and a compile time one. Anything
 * classifying types on both sides of the dispatch boundary, such as a
 * numerical_type_domain, depends on there being only one.
 */
XMIPP4_CONSTEXPR
numerical_type_category get_category(numerical_type type) noexcept;

REXLIB_API
const char* to_string(numerical_type type) noexcept;

REXLIB_API
const char* to_string(numerical_type_category category) noexcept;

REXLIB_API
std::ostream& operator<<(std::ostream &os, numerical_type type);

REXLIB_API
std::ostream& operator<<(std::ostream &os, numerical_type_category category);

} // namespace xmipp4

#include "numerical_type.inl"
