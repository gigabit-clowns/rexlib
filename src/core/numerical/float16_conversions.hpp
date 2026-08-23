// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/numerical/fixed_width_float.hpp>

#include <half.hpp>

namespace rex
{

/**
 * @brief Reinterpret a @ref float16_t as the equivalent @c half_float::half.
 *
 * @ref float16_t is a storage-only placeholder whose bit layout matches
 * @c half_float::half, so the reinterpretation is exact.
 */
half_float::half float16_to_half(float16_t value) noexcept;

/**
 * @brief Reinterpret a @c half_float::half as the equivalent @ref float16_t.
 */
float16_t half_to_float16(half_float::half value) noexcept;

/**
 * @brief Widen a @ref float16_t to a @c float.
 *
 * Defined in a header so that it inlines into hot loops, unlike
 * @ref float16_t 's own out-of-line conversion operator, which delegates here.
 */
float float16_to_float(float16_t value) noexcept;

/**
 * @brief Round a @c float down to a @ref float16_t.
 *
 * Defined in a header so that it inlines into hot loops, unlike
 * @ref float16_t 's own out-of-line converting constructor, which delegates
 * here.
 */
float16_t float_to_float16(float value) noexcept;

} // namespace rex

#include "float16_conversions.inl"
