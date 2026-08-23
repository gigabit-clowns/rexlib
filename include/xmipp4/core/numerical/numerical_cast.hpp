// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"

#include <complex>
#include <type_traits>

namespace rex
{

/**
 * @brief Explicitly cast a numerical value from one type to another.
 *
 * Behaves like @c static_cast<T> for ordinary arithmetic types, but avoids the
 * implicit intermediate conversions that @c static_cast performs when
 * @ref float16_t is involved. As @ref float16_t interoperates with the rest of
 * the world exclusively through @c float, a plain @c static_cast<T> to or from
 * @ref float16_t hides a second, implicit conversion (e.g.
 * @c double -> @c float -> @ref float16_t).
 *
 * The conversion is applied element-wise for @c std::complex operands, so that
 * complex values holding @ref float16_t components are handled as well.
 *
 * @tparam T Destination type.
 * @tparam Q Source type (deduced).
 * @param src Value to convert.
 * @return T The converted value.
 */
template <typename T, typename Q>
REX_NODISCARD
T numerical_cast(const Q &src) noexcept;

} // namespace rex

#include "numerical_cast.inl"
