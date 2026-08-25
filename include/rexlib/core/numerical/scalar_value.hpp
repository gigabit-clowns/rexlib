// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "numerical_type.hpp"
#include "fixed_width_float.hpp"
#include "../memory/byte.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <stdexcept>

namespace rexlib
{

/**
 * @brief Exception thrown when accessing a scalar_value with an incorrect type.
 */
class bad_scalar_value_access
	: public std::exception
{
public:
	using exception::exception;
};

/**
 * @brief Owns a copy of a dynamically typed scalar value.
 *
 * The scalar value must be representable by one of the numerical_type types.
 * Unlike a reference, this class stores the value in-place, so it does not
 * depend on the lifetime of the object it was constructed from.
 */
class scalar_value
{
public:
	/**
	 * @brief Construct a new scalar value object holding a copy of the input.
	 *
	 * @tparam T Type of the value. It must be representable by a numerical_type.
	 * @param value The value to be copied.
	 */
	template <typename T>
	scalar_value(const T &value) noexcept;
	scalar_value(const scalar_value &other) = default;
	scalar_value(scalar_value &&other) = default;
	~scalar_value() = default;

	scalar_value& operator=(const scalar_value &other) = default;
	scalar_value& operator=(scalar_value &&other) = default;

	/**
	 * @brief Type-safe access to the stored value.
	 *
	 * @tparam T Type to be accessed.
	 * @return const T& Reference to the stored value.
	 * @throws bad_scalar_value_access if T does not match the stored type.
	 */
	template <typename T>
	const T& get() const;

	/**
	 * @brief Type-safe, non-throwing access to the stored value.
	 *
	 * @tparam T Type to be accessed.
	 * @return const T* Pointer to the stored value, or nullptr if T does not
	 * match the stored type.
	 */
	template <typename T>
	const T* get_if() const noexcept;

	/**
	 * @brief Get the numerical type of the stored value.
	 *
	 * @return numerical_type The numerical type.
	 */
	numerical_type get_data_type() const noexcept;

private:
	// The largest representable scalar determines the storage size and
	// alignment. All representable scalars are trivially copyable, which makes
	// scalar_value trivially copyable as well.
	using largest_type = std::complex<float64_t>;
	static const std::size_t storage_size = sizeof(largest_type);
	static const std::size_t storage_alignment = alignof(largest_type);

	alignas(storage_alignment) std::array<byte, storage_size> m_storage;
	numerical_type m_data_type;
};

/**
 * @brief Access the value held by a scalar_value with its concrete type.
 *
 * Dispatches on the stored numerical type and calls the provided functor with a
 * const reference to the value, statically typed as the corresponding C++ type.
 * This allows accessing the value in a type-safe manner without knowing its
 * type in advance.
 *
 * The argument order mirrors std::visit: the visitor comes first.
 *
 * @tparam Func Functor type, callable with any of the native scalar types.
 * @param func Functor to be called with the stored value.
 * @param value The scalar value to be visited.
 * @return The result of calling func with the stored value.
 */
template <typename Func>
auto visit(Func &&func, const scalar_value &value);

/**
 * @brief Convert the value held by a scalar_value into a concrete type.
 *
 * Unlike get<T>(), which requires the requested type to match the stored one
 * exactly, this dispatches on the stored numerical type and returns the value
 * converted to @p T following numerical_cast's conversion rules.
 *
 * @tparam T Destination type. Must be one of the numerical_type native types.
 * @param value The scalar value to convert.
 * @return T The stored value converted to T.
 * @throws std::bad_cast if the stored value's type can not be converted into
 * T (e.g. a complex value into a real type).
 */
template <typename T>
T scalar_value_cast(const scalar_value &value);

} // namespace rexlib

#include "scalar_value.inl"
