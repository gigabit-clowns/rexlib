// SPDX-License-Identifier: GPL-3.0-only

#include "strided_axis.hpp"

#include <xmipp4/core/layout/index.hpp>

#include <boost/functional/hash.hpp>

namespace rex
{

REX_INLINE_CONSTEXPR 
strided_axis::strided_axis(std::size_t extent, std::ptrdiff_t stride) noexcept
	: m_extent(extent)
	, m_stride(stride)
{   
}
		
REX_INLINE_CONSTEXPR_CPP20
void strided_axis::swap(strided_axis &other) noexcept
{
	std::swap(m_extent, other.m_extent);
	std::swap(m_stride, other.m_stride);
}

inline
std::size_t strided_axis::hash() const noexcept
{
	std::size_t seed = boost::hash_value(m_extent);
	boost::hash_combine(seed, boost::hash_value(m_stride));
	return seed;
}

REX_INLINE_CONSTEXPR 
void strided_axis::set_extent(std::size_t extent) noexcept
{
	m_extent = extent;
}

REX_INLINE_CONSTEXPR 
std::size_t strided_axis::get_extent() const noexcept
{
	return m_extent;
}

REX_INLINE_CONSTEXPR 
void strided_axis::set_stride(std::ptrdiff_t stride) noexcept
{
	m_stride = stride;
}

REX_INLINE_CONSTEXPR 
std::ptrdiff_t strided_axis::get_stride() const noexcept
{
	return m_stride;
}

REX_INLINE_CONSTEXPR 
std::size_t strided_axis::get_stride_magnitude() const noexcept
{
	return
		m_stride < 0 ? 
		static_cast<std::size_t>(-m_stride) :
		static_cast<std::size_t>(m_stride);
}

REX_INLINE_CONSTEXPR_CPP20
void swap(strided_axis &x, strided_axis &y) noexcept
{
	x.swap(y);
}

REX_INLINE_CONSTEXPR
strided_axis make_contiguous_axis(std::size_t extent) noexcept
{
	return strided_axis(extent, 1);
}

REX_INLINE_CONSTEXPR
strided_axis make_phantom_axis(std::size_t extent) noexcept
{
	return strided_axis(extent, 0);
}

REX_INLINE_CONSTEXPR 
bool compare_strides_equal(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept
{
	return lhs.get_stride_magnitude() == rhs.get_stride_magnitude();
}

REX_INLINE_CONSTEXPR 
bool compare_strides_less(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept
{
	return lhs.get_stride_magnitude() < rhs.get_stride_magnitude();
}

REX_INLINE_CONSTEXPR 
bool compare_strides_greater(
	const strided_axis &lhs, 
	const strided_axis &rhs
) noexcept
{
	return lhs.get_stride_magnitude() > rhs.get_stride_magnitude();
}

REX_INLINE_CONSTEXPR
bool is_contiguous(const strided_axis &axis) noexcept
{
	return axis.get_stride() == 1;
}

REX_INLINE_CONSTEXPR
bool is_contiguous(
	const strided_axis &major,
	const strided_axis &minor
) noexcept
{
	const auto expected = 
		major.get_stride()*static_cast<std::ptrdiff_t>(major.get_extent());
	return expected == minor.get_stride();
}

REX_INLINE_CONSTEXPR
bool is_reversed(const strided_axis &axis) noexcept
{
	return axis.get_stride() < 0;
}

REX_INLINE_CONSTEXPR
bool is_repeating(const strided_axis &axis) noexcept
{
	return axis.get_extent() > 1 && axis.get_stride() == 0;
}

REX_INLINE_CONSTEXPR
bool is_empty(const strided_axis &axis) noexcept
{
	return axis.get_extent() == 0;
}

REX_INLINE_CONSTEXPR
bool is_significant(const strided_axis &axis) noexcept
{
	return axis.get_extent() != 1;
}

REX_INLINE_CONSTEXPR
bool get_axis_last_offset(
	const strided_axis &axis, 
	std::ptrdiff_t &result
) noexcept
{
	const auto extent = axis.get_extent();
	const bool success = (extent > 0);
	if (success)
	{
		const auto stride = axis.get_stride();
		result = (extent-1)*stride;
	}

	return success;
}

REX_INLINE_CONSTEXPR
std::size_t get_axis_pivot_offset(const strided_axis &axis) noexcept
{
	const auto extent = axis.get_extent();
	const auto stride = axis.get_stride();
	return (stride < 0) && (extent > 1) ? (extent-1)*axis.get_stride_magnitude() : 0;
}

REX_INLINE_CONSTEXPR
bool broadcast(strided_axis &axis1, strided_axis &axis2) noexcept
{
	bool result = true;

	if(axis1.get_extent() != axis2.get_extent())
	{
		if(axis1.get_extent() == 1)
		{
			axis1 = make_phantom_axis(axis2.get_extent());
		}
		else if(axis2.get_extent() == 1)
		{
			axis2 = make_phantom_axis(axis1.get_extent());
		}
		else
		{
			result = false; // Unable to broadcast
		}
	}

	return result;
}

REX_INLINE_CONSTEXPR
bool broadcast_to(strided_axis &axis, std::size_t extent) noexcept
{
	bool result = true;

	const auto axis_extent = axis.get_extent();
	if(axis_extent != extent)
	{
		if(axis_extent == 1)
		{
			axis = make_phantom_axis(extent);
		}
		else
		{
			result = false; // Unable to broadcast
		}
	}

	return result;
}

namespace detail
{

REX_INLINE_CONSTEXPR
void apply_index(
	const strided_axis &axis, 
	std::ptrdiff_t &offset,
	std::size_t index
) noexcept
{
	const auto increment = index*axis.get_stride();
	offset += increment;
}

} // namespace

inline
void apply_index(
	const strided_axis &axis, 
	std::ptrdiff_t &offset,
	std::ptrdiff_t index
)
{
	const auto sanitized_index = sanitize_index(index, axis.get_extent());
	detail::apply_index(axis, offset, sanitized_index);
}

namespace detail 
{

REX_INLINE_CONSTEXPR
void apply_slice(
	strided_axis &axis,
	std::ptrdiff_t &offset,
	const slice &slice
) noexcept
{
	const auto start = slice.get_start();
	const auto count = slice.get_count();
	const auto step = slice.get_step();
	const auto stride = axis.get_stride();

	const auto new_stride = stride*step;
	const auto increment = stride*start;

	axis = strided_axis(count, new_stride);
	offset += increment;
}

} // namespace detail

inline
void apply_slice(
	strided_axis &axis,
	std::ptrdiff_t &offset,
	const slice &slice
)
{
	const auto sanitized_slice = sanitize_slice(slice, axis.get_extent());
	detail::apply_slice(axis, offset, sanitized_slice);
}

} // namespace rex
