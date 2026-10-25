// SPDX-License-Identifier: GPL-3.0-only

#include "any_extent_domain.hpp"

namespace rexlib
{

inline
bool any_extent_domain::contains(std::size_t) noexcept
{
	return true;
}

inline
const char* any_extent_domain::get_description() noexcept
{
	return "any extent";
}

} // namespace rexlib
