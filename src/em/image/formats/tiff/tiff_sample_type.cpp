// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_sample_type.hpp"

#include <tiff.h>

namespace rexlib
{
namespace em
{
namespace tiff
{

numerical_type get_data_type(
	std::uint16_t bits_per_sample,
	std::uint16_t sample_format
) noexcept
{
	switch (sample_format)
	{
	case SAMPLEFORMAT_UINT:
		switch (bits_per_sample)
		{
		case 8: return numerical_type::uint8;
		case 16: return numerical_type::uint16;
		case 32: return numerical_type::uint32;
		case 64: return numerical_type::uint64;
		default: return numerical_type::unknown;
		}
	case SAMPLEFORMAT_INT:
		switch (bits_per_sample)
		{
		case 8: return numerical_type::int8;
		case 16: return numerical_type::int16;
		case 32: return numerical_type::int32;
		case 64: return numerical_type::int64;
		default: return numerical_type::unknown;
		}
	case SAMPLEFORMAT_IEEEFP:
		switch (bits_per_sample)
		{
		case 16: return numerical_type::float16;
		case 32: return numerical_type::float32;
		case 64: return numerical_type::float64;
		default: return numerical_type::unknown;
		}
	case SAMPLEFORMAT_COMPLEXIEEEFP:
		switch (bits_per_sample)
		{
		case 64: return numerical_type::complex_float32;
		case 128: return numerical_type::complex_float64;
		default: return numerical_type::unknown;
		}
	default:
		return numerical_type::unknown;
	}
}

bool is_supported(numerical_type type) noexcept
{
	return get_sample_format(type) != 0;
}

std::uint16_t get_bits_per_sample(numerical_type type) noexcept
{
	if (!is_supported(type))
	{
		return 0;
	}

	return static_cast<std::uint16_t>(get_size(type) * 8);
}

std::uint16_t get_sample_format(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::uint8:
	case numerical_type::uint16:
	case numerical_type::uint32:
	case numerical_type::uint64:
		return SAMPLEFORMAT_UINT;
	case numerical_type::int8:
	case numerical_type::int16:
	case numerical_type::int32:
	case numerical_type::int64:
		return SAMPLEFORMAT_INT;
	case numerical_type::float16:
	case numerical_type::float32:
	case numerical_type::float64:
		return SAMPLEFORMAT_IEEEFP;
	case numerical_type::complex_float32:
	case numerical_type::complex_float64:
		return SAMPLEFORMAT_COMPLEXIEEEFP;
	default:
		return 0;
	}
}

} // namespace tiff
} // namespace em
} // namespace rexlib
