// SPDX-License-Identifier: GPL-3.0-only

#include "alternating_sign.hpp"

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
std::complex<T> alternating_sign::apply(
	std::complex<T> value,
	std::ptrdiff_t x,
	std::ptrdiff_t stored_y,
	std::ptrdiff_t stored_z
) const noexcept
{
	return ((x + stored_y + stored_z) % 2 == 0) ? value : -value;
}

} // namespace cpu
} // namespace rexlib
