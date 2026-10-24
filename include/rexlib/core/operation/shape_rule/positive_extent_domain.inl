// SPDX-License-Identifier: GPL-3.0-only

#include "positive_extent_domain.hpp"

namespace rexlib
{

inline
bool positive_extent_domain::contains(std::size_t extent) noexcept
{
	return extent > 0;
}

inline
const char* positive_extent_domain::get_description() noexcept
{
	return "at least one";
}

} // namespace rexlib
