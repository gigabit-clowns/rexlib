// SPDX-License-Identifier: GPL-3.0-only

#include "same_extent_transform.hpp"

namespace rexlib
{

inline
std::size_t same_extent_transform::apply(std::size_t extent) noexcept
{
	return extent;
}

inline
const char* same_extent_transform::get_relation() noexcept
{
	return "the extent of";
}

} // namespace rexlib
