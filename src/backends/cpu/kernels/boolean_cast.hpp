// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace cpu
{

/**
 * @brief Read an element as a truth value.
 *
 * An element is true when it differs from the zero of its own type. That
 * one reading covers every type an operation may admit: a complex number
 * is false only when both of its parts are, since its zero is the origin,
 * and comparing against a typed zero avoids the conversion to bool that
 * some compilers warn about.
 *
 * @tparam T The element type.
 * @param value The element.
 * @return bool Whether the element is true.
 */
template <typename T>
bool to_boolean(const T &value) noexcept
{
	return value != T(0);
}

} // namespace cpu
} // namespace rexlib
