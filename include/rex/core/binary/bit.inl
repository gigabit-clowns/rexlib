// SPDX-License-Identifier: GPL-3.0-only

#include "bit.hpp"
#include "../platform/cpp_features.hpp"

#include <limits>
#if REXLIB_HAS_LIB_BITOPS
		#include <bit>
#endif

namespace rex
{

REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
std::size_t bit(std::size_t i) noexcept
{
	REXLIB_CONSTEXPR auto n_bits = std::numeric_limits<std::size_t>::digits;
	if (i >= n_bits)
	{
		return 0;
	}
	else
	{
		return std::size_t(1) << i;
	}
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_range_mask(std::size_t start, std::size_t end) noexcept
{
	return T(bit(end)) - T(bit(start));
}

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T&>::type
set_lowest_zero_inplace(T& x) noexcept
{
	x |= x + 1;
	return x;
}

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T&>::type
clear_lowest_one_inplace(T& x) noexcept
{
	x &= x - 1;
	return x;
}

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
mask_trailing_zeros(T x) noexcept
{
	return ~x & (x - 1);
}

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
mask_trailing_ones(T x) noexcept
{
	return x & (~x - 1);
}

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
rotate_left(T x, int s) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::rotl(x, s);
#else
	if (s >= 0)
	{
		REXLIB_CONST_CONSTEXPR auto N = std::numeric_limits<T>::digits; 
		const auto r = static_cast<unsigned>(s) % N;
		if(r > 0)
			return (x << r) | (x >> (N - r)); 
		else
			return x;
	}
	else
	{
		return rotate_right(x, -s);
	}
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
rotate_right(T x, int s) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::rotr(x, s);
#else
	if (s >= 0)
	{
		REXLIB_CONST_CONSTEXPR auto N = std::numeric_limits<T>::digits; 
		const auto r = static_cast<unsigned>(s) % N;
		if(r > 0)
			return (x >> r) | (x << (N - r)); 
		else
			return x;
	}
	else
	{
		return rotate_left(x, -s);
	}
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
has_single_bit(T x) noexcept
{
#if REXLIB_HAS_LIB_POW2
	return std::has_single_bit(x);
#else
	if(x)
	{
		// If it has a single one, it should be zero
		// after clearing its lowest one
		clear_lowest_one_inplace(x); 
		return !x;
	}
	else
	{
		return false;
	}
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_floor(T x) noexcept
{
#if REXLIB_HAS_LIB_POW2
	return std::bit_floor(x);
#else
	if(x) 
	{
		return T(1) << (bit_width(x) - 1);
	}
	else
	{
		return T(0);
	}
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, T>::type
bit_ceil(T x) noexcept
{
#if REXLIB_HAS_LIB_POW2
	return std::bit_ceil(x);
#else
	return x <= T(1) ? T(1) : (T(1) << bit_width(T(x - 1)));
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
bit_width(T x) noexcept
{
#if REXLIB_HAS_LIB_POW2
	return std::bit_width(x);
#else
	REXLIB_CONST_CONSTEXPR auto N = std::numeric_limits<T>::digits; 
	return N - count_leading_zeros(x); 
#endif
}

namespace detail
{

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_leading_zeros(T x) noexcept
{
	REXLIB_CONSTEXPR auto N = std::numeric_limits<T>::digits;

	auto count = N;
	for(auto n = N >> 1; n; n >>= 1)
	{
		const auto y = T(x >> n);
		if(y)
		{
			count -= n;
			x = y;
		}
	}

	return x ? (count-1) : count;
}

#if REXLIB_HAS_BUILTIN(clz)
REXLIB_INLINE_CONSTEXPR unsigned int
count_leading_zeros(unsigned int x) noexcept
{
	// __builtin_clz is not defined for zero
	if(!x)
	{
		return std::numeric_limits<unsigned int>::digits;
	}

	return REXLIB_BUILTIN(clz)(x);
}
#endif

#if REXLIB_HAS_BUILTIN(clzll)
REXLIB_INLINE_CONSTEXPR unsigned int
count_leading_zeros(unsigned long long x) noexcept
{
	// __builtin_clzll is not defined for zero
	if(!x)
	{
		return std::numeric_limits<unsigned long long>::digits;
	}

	return REXLIB_BUILTIN(clzll)(x);
}
#endif

} // namespace detail

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_leading_zeros(T x) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::countl_zero(x);
#else
	return detail::count_leading_zeros(x);
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_leading_ones(T x) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::countl_one(x);
#else
	return count_leading_zeros(T(~x));
#endif
}

namespace detail
{

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_trailing_zeros(T x) noexcept
{
	REXLIB_CONSTEXPR auto N = std::numeric_limits<T>::digits;

	auto count = N;
	for(auto n = N >> 1; n; n >>= 1)
	{
		const auto y = T(x << n);
		if(y)
		{
			count -= n;
			x = y;
		}
	}

	return x ? (count-1) : count;
}

#if REXLIB_HAS_BUILTIN(ctz)
REXLIB_INLINE_CONSTEXPR unsigned int
count_trailing_zeros(unsigned int x) noexcept
{
	// __builtin_ctz is not defined for zero
	if(x == 0)
	{
		return std::numeric_limits<unsigned int>::digits;
	}

	return REXLIB_BUILTIN(ctz)(x);
}
#endif

#if REXLIB_HAS_BUILTIN(ctzll)
REXLIB_INLINE_CONSTEXPR unsigned int
count_trailing_zeros(unsigned long long x) noexcept
{
	// __builtin_ctzll is not defined for zero
	if(x == 0)
	{
		return std::numeric_limits<unsigned long long>::digits;
	}

	return REXLIB_BUILTIN(ctzll)(x);
}
#endif

} // namespace detail

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_trailing_zeros(T x) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::countr_zero(x);
#else
	return detail::count_trailing_zeros(x);
#endif
}

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
count_trailing_ones(T x) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::countr_one(x);
#else
	return count_trailing_zeros(T(~x));
#endif
}

namespace detail
{

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
popcount(T x) noexcept
{
	int count = 0;
	while(x)
	{
		++count;
		clear_lowest_one_inplace(x);
	}
	return count;
}

#if REXLIB_HAS_BUILTIN(popcount)
REXLIB_INLINE_CONSTEXPR int
popcount(unsigned int x) noexcept
{
	return REXLIB_BUILTIN(popcount)(x);
}
#endif

#if REXLIB_HAS_BUILTIN(popcountll)
REXLIB_INLINE_CONSTEXPR int
popcount(unsigned long long x) noexcept
{
	return REXLIB_BUILTIN(popcountll)(x);
}
#endif

} // namespace detail

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, int>::type
popcount(T x) noexcept
{
#if REXLIB_HAS_LIB_BITOPS
	return std::popcount(x);
#else
	return detail::popcount(x);
#endif
}

namespace detail
{

template<typename T>
REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
parity(T x) noexcept
{
	bool parity = false;
	while(x)
	{
		parity = !parity;
		clear_lowest_one_inplace(x);
	}
	return parity;
}

#if REXLIB_HAS_BUILTIN(parity)
REXLIB_INLINE_CONSTEXPR bool
parity(unsigned int x) noexcept
{
	return REXLIB_BUILTIN(parity)(x); 
}
#endif

#if REXLIB_HAS_BUILTIN(parityll)
REXLIB_INLINE_CONSTEXPR bool
parity(unsigned long long x) noexcept
{
	return REXLIB_BUILTIN(parityll)(x); 
}
#endif

} // namespace detail

template<typename T>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR 
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
parity(T x) noexcept
{
	return detail::parity(x);
}

} // namespace rex
