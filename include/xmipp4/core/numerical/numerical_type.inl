// SPDX-License-Identifier: GPL-3.0-only

#include "numerical_type.hpp"

namespace xmipp4
{

REX_INLINE_CONSTEXPR
numerical_type_category get_category(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::boolean:
		return numerical_type_category::boolean;
	case numerical_type::char8:
		return numerical_type_category::character;
	case numerical_type::int8:
	case numerical_type::int16:
	case numerical_type::int32:
	case numerical_type::int64:
		return numerical_type_category::signed_integer;
	case numerical_type::uint8:
	case numerical_type::uint16:
	case numerical_type::uint32:
	case numerical_type::uint64:
		return numerical_type_category::unsigned_integer;
	case numerical_type::float16:
	case numerical_type::float32:
	case numerical_type::float64:
		return numerical_type_category::floating_point;
	case numerical_type::complex_float16:
	case numerical_type::complex_float32:
	case numerical_type::complex_float64:
		return numerical_type_category::complex;
	default:
		return numerical_type_category::unknown;
	}
}

} // namespace xmipp4
