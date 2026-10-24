// SPDX-License-Identifier: GPL-3.0-only

#include "half_spectrum_extent_transform.hpp"

namespace rexlib
{
namespace ops
{

inline
std::size_t half_spectrum_extent_transform::apply(std::size_t extent) noexcept
{
	return extent / 2 + 1;
}

inline
const char* half_spectrum_extent_transform::get_relation() noexcept
{
	return "the half spectrum extent of";
}

} // namespace ops
} // namespace rexlib
