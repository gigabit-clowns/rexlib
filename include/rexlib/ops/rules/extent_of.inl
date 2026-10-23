// SPDX-License-Identifier: GPL-3.0-only

#include "extent_of.hpp"

namespace rexlib
{
namespace ops
{

template <typename Parameters>
inline
std::size_t extent_of::get(const Parameters &parameters) noexcept
{
	return parameters.get_extent();
}

} // namespace ops
} // namespace rexlib
