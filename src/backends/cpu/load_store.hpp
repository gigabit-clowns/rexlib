// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <type_traits>
#include <utility>

namespace rex
{
namespace cpu
{

/**
 * @brief Read an operand element in the type used to compute with it.
 *
 * The general case: the storage type is its own compute type, so the element
 * is returned unchanged.
 *
 * @tparam T Storage (and compute) element type.
 * @param ptr Pointer to the element to read.
 * @return The value of the element.
 */
template <typename T>
T load(const T *ptr) noexcept;

/**
 * @brief Read a @ref float16_t element as a @c float.
 *
 * @ref float16_t is a storage-only type without arithmetic support, so it is
 * widened to @c float to be computed with. The result is rounded back by the
 * matching @ref store overload.
 *
 * @param ptr Pointer to the element to read.
 * @return The element widened to @c float.
 */
float load(const float16_t *ptr) noexcept;

/**
 * @brief Read a @c complex<float16_t> element as a @c complex<float>.
 *
 * Widens each @ref float16_t component to @c float, as the scalar overload
 * does.
 *
 * @param ptr Pointer to the element to read.
 * @return The element widened to @c complex<float>.
 */
std::complex<float> load(const std::complex<float16_t> *ptr) noexcept;

/**
 * @brief Write a computed value into an operand element.
 *
 * The general case: the value is assigned to the element, converting to the
 * storage type if needed.
 *
 * @tparam T Storage element type.
 * @tparam U Type of the computed value.
 * @param ptr Pointer to the element to write.
 * @param value Value to store.
 */
template <typename T, typename U>
void store(T *ptr, const U &value) noexcept;

/**
 * @brief Write a @c float into a @ref float16_t element.
 *
 * Rounds the single-precision result back to @ref float16_t, inverting the
 * @ref load overload that widened it.
 *
 * @param ptr Pointer to the element to write.
 * @param value Value to round and store.
 */
void store(float16_t *ptr, float value) noexcept;

/**
 * @brief Write a @c complex<float> into a @c complex<float16_t> element.
 *
 * Rounds each component back to @ref float16_t, as the scalar overload does.
 *
 * @param ptr Pointer to the element to write.
 * @param value Value to round and store.
 */
void store(
	std::complex<float16_t> *ptr,
	const std::complex<float> &value
) noexcept;

/**
 * @brief Convert a source element into a destination element.
 *
 * The general case: assigns the source to the destination with an explicit
 * @c static_cast. Same-representation @ref float16_t copies take this path and
 * keep their bits without going through @c float.
 *
 * @tparam T Destination storage type.
 * @tparam Q Source storage type.
 * @param destination Pointer to the element to write.
 * @param source Pointer to the element to read.
 */
template <typename T, typename Q>
void cast(T *destination, const Q *source) noexcept;

/**
 * @brief Convert an arithmetic value into a @ref float16_t element.
 *
 * Rounds through @c float inline instead of through @ref float16_t 's
 * out-of-line converting constructor.
 *
 * @tparam Q Arithmetic source type.
 * @param destination Pointer to the element to write.
 * @param source Pointer to the element to read.
 */
template <typename Q>
typename std::enable_if<std::is_arithmetic<Q>::value>::type
cast(float16_t *destination, const Q *source) noexcept;

/**
 * @brief Convert a @ref float16_t element into an arithmetic value.
 *
 * Widens through @c float inline instead of through @ref float16_t 's
 * out-of-line conversion operator.
 *
 * @tparam T Arithmetic destination type.
 * @param destination Pointer to the element to write.
 * @param source Pointer to the element to read.
 */
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
cast(T *destination, const float16_t *source) noexcept;

/**
 * @brief Convert a complex element into a complex element, component-wise.
 *
 * Each component is converted through the scalar overloads, so
 * @c complex<float16_t> components keep their representation or bridge
 * @c float inline as needed.
 *
 * @tparam T Destination component type.
 * @tparam Q Source component type.
 * @param destination Pointer to the element to write.
 * @param source Pointer to the element to read.
 */
template <typename T, typename Q>
void cast(std::complex<T> *destination, const std::complex<Q> *source) noexcept;

/**
 * @brief Convert a scalar element into a complex element.
 *
 * The scalar becomes the real part; the imaginary part is set to zero.
 *
 * @tparam T Destination component type.
 * @tparam Q Scalar source type.
 * @param destination Pointer to the element to write.
 * @param source Pointer to the element to read.
 */
template <typename T, typename Q>
void cast(std::complex<T> *destination, const Q *source) noexcept;

/**
 * @brief The type an element of storage type T is computed in.
 *
 * Reads the answer off @ref load rather than restating it, so that the
 * widening rule has one definition: half precision has no arithmetic of its
 * own and is widened on load and rounded back on store, and every other type
 * is computed in itself.
 *
 * Anything holding an intermediate result, such as a reduction accumulator
 * or the running value of a generated sequence, is typed on this rather than
 * on the storage type, which is what keeps the rounding to the single step
 * that writes the element.
 *
 * @tparam T The storage element type.
 */
template <typename T>
struct element_compute_type
{
	using type = decltype(load(std::declval<const T*>()));
};

} // namespace cpu
} // namespace rex

#include "load_store.inl"
