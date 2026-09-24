// SPDX-License-Identifier: GPL-3.0-only

#include "mrc_mode.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>

namespace rexlib
{
namespace em
{
namespace mrc
{

bool is_supported(mrc_mode mode) noexcept
{
	return get_data_type(mode, true) != numerical_type::unknown;
}

numerical_type get_data_type(mrc_mode mode, bool signed_bytes) noexcept
{
	switch (mode)
	{
	case mrc_mode::int8:
		return signed_bytes ? numerical_type::int8 : numerical_type::uint8;
	case mrc_mode::int16: return numerical_type::int16;
	case mrc_mode::float32: return numerical_type::float32;
	case mrc_mode::complex_float32: return numerical_type::complex_float32;
	case mrc_mode::uint16: return numerical_type::uint16;
	case mrc_mode::float16: return numerical_type::float16;
	default: return numerical_type::unknown;
	}
}

std::size_t get_element_size(mrc_mode mode) noexcept
{
	const auto type = get_data_type(mode, true);
	if (type == numerical_type::unknown)
	{
		return 0;
	}

	return get_size(type);
}

mrc_mode get_mode(numerical_type type)
{
	switch (type)
	{
	case numerical_type::int8:
	case numerical_type::uint8:
		return mrc_mode::int8;
	case numerical_type::int16: return mrc_mode::int16;
	case numerical_type::float32: return mrc_mode::float32;
	case numerical_type::complex_float32: return mrc_mode::complex_float32;
	case numerical_type::uint16: return mrc_mode::uint16;
	case numerical_type::float16: return mrc_mode::float16;
	default:
		throw unsupported_operation_error(
			"mrc::get_mode: The MRC format has no mode for this data type."
		);
	}
}

bool needs_imod_unsigned_flag(numerical_type type) noexcept
{
	return type == numerical_type::uint8;
}

} // namespace mrc
} // namespace em
} // namespace rexlib
