// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rex
{

/**
 * @brief Helper class to pass type information to a function.
 *
 * Eventually to be replaced with `std::type_identity` in C++20.
 *
 * @tparam T Type passed.
 */
template <typename T>
class type_tag
{
public:
	using type = T; ///< The type passed as the template argument.
};

} // namespace rex
