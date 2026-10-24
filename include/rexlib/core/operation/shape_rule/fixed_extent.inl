// SPDX-License-Identifier: GPL-3.0-only

#include "fixed_extent.hpp"

namespace rexlib
{

template <std::size_t N>
template <typename Parameters>
inline
std::size_t fixed_extent<N>::get(const Parameters&) noexcept
{
	return N;
}

} // namespace rexlib
