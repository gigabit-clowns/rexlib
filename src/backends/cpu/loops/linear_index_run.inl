// SPDX-License-Identifier: GPL-3.0-only

#include "linear_index_run.hpp"

namespace rexlib
{
namespace cpu
{

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
linear_index_run<Stride>::linear_index_run(
	std::ptrdiff_t index,
	Stride stride
) noexcept
	: m_index(index)
	, m_stride(stride)
{
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
std::size_t linear_index_run<Stride>::get_index() const noexcept
{
	return static_cast<std::size_t>(m_index);
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
linear_index_run<Stride>
linear_index_run<Stride>::rebased(
	const std::ptrdiff_t *index_offsets
) const noexcept
{
	return linear_index_run(index_offsets[0], m_stride);
}

template <typename Stride>
REXLIB_INLINE_CONSTEXPR
linear_index_run<Stride>
linear_index_run<Stride>::advanced(std::size_t count) const noexcept
{
	const auto step = static_cast<std::ptrdiff_t>(m_stride);
	return linear_index_run(
		m_index + static_cast<std::ptrdiff_t>(count)*step,
		m_stride
	);
}

} // namespace cpu
} // namespace rexlib
