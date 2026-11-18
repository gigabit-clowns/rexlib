// SPDX-License-Identifier: GPL-3.0-only

#include "tabulated_phase.hpp"

#include <backends/cpu/em/skeletons/slice_insertion/axis_phase.hpp>

namespace rexlib
{
namespace cpu
{

template <typename T>
inline
tabulated_phase<T>::tabulated_phase(std::size_t extent)
	: m_x(extent / 2 + 1)
	, m_full(extent)
{
	const auto half = extent / 2;
	const auto shift = -static_cast<double>(half);
	fill_axis_phase(m_x.data(), m_x.size(), extent, shift);
	fill_axis_phase(m_full.data(), m_full.size(), extent, shift);
}

template <typename T>
inline
std::complex<T> tabulated_phase<T>::apply(
	std::complex<T> value,
	std::ptrdiff_t x,
	std::ptrdiff_t stored_y,
	std::ptrdiff_t stored_z
) const noexcept
{
	return value *
		m_x[static_cast<std::size_t>(x)] *
		m_full[static_cast<std::size_t>(stored_y)] *
		m_full[static_cast<std::size_t>(stored_z)];
}

} // namespace cpu
} // namespace rexlib
