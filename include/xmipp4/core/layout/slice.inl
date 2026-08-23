// SPDX-License-Identifier: GPL-3.0-only

#include "slice.hpp"

#include <sstream>
#include <limits>
#include <exception>

namespace rex 
{

REX_INLINE_CONSTEXPR 
slice::slice(start_type start, count_type count, step_type step ) noexcept
	: m_start(start)
	, m_count(count)
	, m_step(step)
{
}

REX_INLINE_CONSTEXPR 
void slice::set_start(start_type start) noexcept
{
	m_start = start;
}

REX_INLINE_CONSTEXPR 
const slice::start_type& slice::get_start() const noexcept
{
	return m_start;
}

REX_INLINE_CONSTEXPR 
void slice::set_count(count_type count) noexcept
{
	m_count = count;
}

REX_INLINE_CONSTEXPR 
const slice::count_type& slice::get_count() const noexcept
{
	return m_count;
}

REX_INLINE_CONSTEXPR 
void slice::set_step(step_type step) noexcept
{
	m_step = step;
}

REX_INLINE_CONSTEXPR 
const typename slice::step_type& slice::get_step() const noexcept
{
	return m_step;
}

REX_INLINE_CONSTEXPR std::size_t end() noexcept
{
	return std::numeric_limits<std::size_t>::max();
}

REX_INLINE_CONSTEXPR slice all() noexcept
{
	return slice(0, end(), 1);
}

REX_INLINE_CONSTEXPR slice even() noexcept
{
	return slice(0, end(), 2);
}

REX_INLINE_CONSTEXPR slice odd() noexcept
{
	return slice(1, end(), 2);
}

REX_INLINE_CONSTEXPR 
slice make_slice(std::size_t count) noexcept
{
	return slice(0, count, 1);
}

REX_INLINE_CONSTEXPR 
slice make_slice(std::ptrdiff_t start, std::size_t count) noexcept
{
	return slice(start, count, 1);
}

REX_INLINE_CONSTEXPR 
slice make_slice(
	std::ptrdiff_t start, 
	std::size_t count, 
	std::ptrdiff_t step
) noexcept
{
	return slice(start, count, step);
}

namespace detail
{

inline
void sanitize_slice_start(slice &slice, std::size_t extent)
{
	const auto start = slice.get_start();
	const auto count = slice.get_count();
	const auto step = slice.get_step();

	if (start < -static_cast<std::ptrdiff_t>(extent))
	{
		std::ostringstream oss;
		oss << "Slice's negative start index " << start 
			<< " is out of bounds for extent " << extent << ".";
		throw std::out_of_range(oss.str());
	}

	if (step > 0 || count == 0)
	{
		if (start > static_cast<std::ptrdiff_t>(extent) )
		{
			std::ostringstream oss;
			oss << "Slice's start index " << start 
				<< " is out of bounds for extent " << extent << ".";
			throw std::out_of_range(oss.str());
		}
	}
	else
	{
		if (start >= static_cast<std::ptrdiff_t>(extent) )
		{
			std::ostringstream oss;
			oss << "Backwards slice's start index " << start 
				<< " is out of bounds for extent " << extent << ".";
			throw std::out_of_range(oss.str());
		}
	}

	if (start < 0)
	{
		slice.set_start(start + extent);
	}
}

inline
void sanitize_slice_count(slice &slice, std::size_t extent)
{
	const auto start = slice.get_start();
	const auto count = slice.get_count();
	const auto step = slice.get_step();

	if (step > 0)
	{
		if (count == end())
		{
			const auto new_count = (extent - start + step - 1) / step;
			slice.set_count(new_count);
		}
		else if (count > 0U && start + step*(count-1) >= extent)
		{
			std::ostringstream oss;
			oss << "Slice count " << count 
				<< " start index " << start
				<< " and step " << step 
				<< " overflows extent " << extent;
			throw std::out_of_range(oss.str());
		}
	}
	else // step < 0
	{
		const auto abs_step = -step;
		if (count == end())
		{
			const auto new_count = start / abs_step + 1;
			slice.set_count(new_count);
		}
		else if (count > 0U && abs_step*static_cast<std::ptrdiff_t>(count-1) > start)
		{
			std::ostringstream oss;
			oss << "Reversed slice with count " << count 
				<< " start index " << start
				<< " and step " << step 
				<< " underflows 0";
			throw std::out_of_range(oss.str());
		}
	}
}

inline
void sanitize_slice_step(const slice &slice)
{
	if (slice.get_step() == 0)
	{
		throw std::invalid_argument("Slice step cannot be zero.");
	}
}

} // namespace detail

inline
slice sanitize_slice(const slice& slice, std::size_t extent)
{
	auto result = slice;

	detail::sanitize_slice_step(result);
	detail::sanitize_slice_start(result, extent);
	detail::sanitize_slice_count(result, extent);

	return result;
}

} // namespace rex
