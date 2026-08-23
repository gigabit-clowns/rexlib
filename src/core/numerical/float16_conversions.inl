// SPDX-License-Identifier: GPL-3.0-only

#include "float16_conversions.hpp"

#include <cstring>

namespace rex
{

inline half_float::half float16_to_half(float16_t value) noexcept
{
	half_float::half result;
	static_assert(
		sizeof(result) == sizeof(value),
		"float16_t and half_float::half must share the same size"
	);
	std::memcpy(
		static_cast<void*>(&result),
		static_cast<const void*>(&value),
		sizeof(result)
	);
	return result;
}

inline float16_t half_to_float16(half_float::half value) noexcept
{
	float16_t result;
	static_assert(
		sizeof(result) == sizeof(value),
		"float16_t and half_float::half must share the same size"
	);
	std::memcpy(
		static_cast<void*>(&result),
		static_cast<const void*>(&value),
		sizeof(result)
	);
	return result;
}

inline float float16_to_float(float16_t value) noexcept
{
	return static_cast<float>(float16_to_half(value));
}

inline float16_t float_to_float16(float value) noexcept
{
	return half_to_float16(half_float::half(value));
}

} // namespace rex
