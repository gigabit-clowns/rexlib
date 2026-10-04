// SPDX-License-Identifier: GPL-3.0-only

#include "tiff_sample_type.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>

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
		default: return numerical_type::unknown;
		}
	case SAMPLEFORMAT_INT:
		switch (bits_per_sample)
		{
		case 8: return numerical_type::int8;
		case 16: return numerical_type::int16;
		default: return numerical_type::unknown;
		}
	case SAMPLEFORMAT_IEEEFP:
		switch (bits_per_sample)
		{
		case 16: return numerical_type::float16;
		case 32: return numerical_type::float32;
		default: return numerical_type::unknown;
		}
	default:
		return numerical_type::unknown;
	}
}

bool is_supported(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::int8:
	case numerical_type::uint8:
	case numerical_type::int16:
	case numerical_type::uint16:
	case numerical_type::float16:
	case numerical_type::float32:
		return true;
	default:
		return false;
	}
}

std::uint16_t get_bits_per_sample(numerical_type type)
{
	if (!is_supported(type))
	{
		throw unsupported_operation_error(
			"tiff::get_bits_per_sample: The TIFF format has no sample this "
			"format transfers for this data type."
		);
	}

	return static_cast<std::uint16_t>(get_size(type) * 8);
}

std::uint16_t get_sample_format(numerical_type type)
{
	switch (type)
	{
	case numerical_type::uint8:
	case numerical_type::uint16:
		return SAMPLEFORMAT_UINT;
	case numerical_type::int8:
	case numerical_type::int16:
		return SAMPLEFORMAT_INT;
	case numerical_type::float16:
	case numerical_type::float32:
		return SAMPLEFORMAT_IEEEFP;
	default:
		throw unsupported_operation_error(
			"tiff::get_sample_format: The TIFF format has no sample this "
			"format transfers for this data type."
		);
	}
}

} // namespace tiff
} // namespace em
} // namespace rexlib
