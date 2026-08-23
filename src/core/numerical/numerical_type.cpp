// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/numerical/numerical_type.hpp>

#include <xmipp4/core/platform/enum_helpers.hpp>
#include <xmipp4/core/numerical/fixed_width_float.hpp>

#include "numerical_type_promotion_lattice.hpp"

#include <complex>

namespace rex
{

namespace
{

using numerical_type_promotion_row =
	std::array<numerical_type, static_cast<std::size_t>(numerical_type::count)>;

using numerical_type_promotion_table =
	std::array<
		numerical_type_promotion_row, 
		static_cast<std::size_t>(numerical_type::count)
	>;

REX_INLINE_CONSTEXPR_CPP23
numerical_type_promotion_lattice
compute_numerical_type_promotion_lattice() noexcept
{
	numerical_type_promotion_lattice lattice;

	// Lattice based on JAX's type promotion mechanism
	// https://docs.jax.dev/en/latest/jep/9407-type-promotion.html
	lattice.add_edge(numerical_type::boolean, numerical_type::uint8);
	lattice.add_edge(numerical_type::boolean, numerical_type::int8);

	lattice.add_edge(numerical_type::char8, numerical_type::int16);

	lattice.add_edge(numerical_type::uint8, numerical_type::uint16);
	lattice.add_edge(numerical_type::uint8, numerical_type::int16);
	lattice.add_edge(numerical_type::int8, numerical_type::int16);
	lattice.add_edge(numerical_type::uint16, numerical_type::uint32);
	lattice.add_edge(numerical_type::uint16, numerical_type::int32);
	lattice.add_edge(numerical_type::int16, numerical_type::int32);
	lattice.add_edge(numerical_type::uint32, numerical_type::uint64);
	lattice.add_edge(numerical_type::uint32, numerical_type::int64);
	lattice.add_edge(numerical_type::int32, numerical_type::int64);

	// Unlike JAX, uint64 may be promoted to int64.
	lattice.add_edge(numerical_type::uint64, numerical_type::int64);

	lattice.add_edge(numerical_type::int64, numerical_type::float16);

	lattice.add_edge(numerical_type::float16, numerical_type::float32);
	lattice.add_edge(numerical_type::float16, numerical_type::complex_float16);
	lattice.add_edge(numerical_type::float32, numerical_type::float64);
	lattice.add_edge(numerical_type::float32, numerical_type::complex_float32);
	lattice.add_edge(numerical_type::float64, numerical_type::complex_float64);

	lattice.add_edge(
		numerical_type::complex_float16, 
		numerical_type::complex_float32
	);
	lattice.add_edge(
		numerical_type::complex_float32, 
		numerical_type::complex_float64
	);

	return lattice;
}

REX_INLINE_CONSTEXPR_CPP23
numerical_type_promotion_table 
compute_numerical_type_promotion_table() noexcept
{
	numerical_type_promotion_table table = {};
	const auto lattice = compute_numerical_type_promotion_lattice();

	for (std::size_t i = 1; i < table.size(); ++i)
	{
		for (std::size_t j = 0; j < i; ++j)
		{
			const auto supremum = lattice.get_supremum(
				static_cast<numerical_type>(i),
				static_cast<numerical_type>(j)
			);

			table[i][j] = supremum;
			table[j][i] = supremum;
		}
	}

	for (std::size_t i = 1; i < table.size(); ++i)
	{
		table[i][i] = static_cast<numerical_type>(i); // Trivial case
	}

	return table;
}

} // anonymous namespace

std::size_t get_size(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::boolean: return sizeof(bool);
	case numerical_type::char8: return sizeof(char);
	case numerical_type::int8: return sizeof(std::int8_t);
	case numerical_type::uint8: return sizeof(std::int8_t);
	case numerical_type::int16: return sizeof(std::int16_t);
	case numerical_type::uint16: return sizeof(std::uint16_t);
	case numerical_type::int32: return sizeof(std::int32_t);
	case numerical_type::uint32: return sizeof(std::uint32_t);
	case numerical_type::int64: return sizeof(std::int64_t);
	case numerical_type::uint64: return sizeof(std::uint64_t);
	case numerical_type::float16: return sizeof(float16_t);
	case numerical_type::float32: return sizeof(float32_t);
	case numerical_type::float64: return sizeof(float64_t);
	case numerical_type::complex_float16: return sizeof(std::complex<float16_t>);
	case numerical_type::complex_float32: return sizeof(std::complex<float32_t>);
	case numerical_type::complex_float64: return sizeof(std::complex<float64_t>);
	default: return 0UL;
	}
}

numerical_type make_complex(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::float16: return numerical_type::complex_float16;
	case numerical_type::float32: return numerical_type::complex_float32;
	case numerical_type::float64: return numerical_type::complex_float64;
	case numerical_type::complex_float16: 
		return numerical_type::complex_float16;
	case numerical_type::complex_float32: 
		return numerical_type::complex_float32;
	case numerical_type::complex_float64: 
		return numerical_type::complex_float64;
	default: return numerical_type::unknown;
	} 
}

numerical_type make_real(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::boolean:
	case numerical_type::char8:
	case numerical_type::int8:
	case numerical_type::uint8:
	case numerical_type::int16:
	case numerical_type::uint16:
	case numerical_type::int32:
	case numerical_type::uint32:
	case numerical_type::int64:
	case numerical_type::uint64:
	case numerical_type::float16:
	case numerical_type::float32:
	case numerical_type::float64:
		return type;

	case numerical_type::complex_float16: return numerical_type::float16;
	case numerical_type::complex_float32: return numerical_type::float32;
	case numerical_type::complex_float64: return numerical_type::float64;

	default: return numerical_type::unknown;
	}
}

numerical_type make_inexact(numerical_type type) noexcept
{
	switch (type)
	{
	case numerical_type::boolean:
	case numerical_type::int8:
	case numerical_type::uint8:
	case numerical_type::int16:
	case numerical_type::uint16:
	case numerical_type::int32:
	case numerical_type::uint32:
	case numerical_type::int64:
	case numerical_type::uint64:
		return numerical_type::float64;

	case numerical_type::float16:
	case numerical_type::float32:
	case numerical_type::float64:
	case numerical_type::complex_float16:
	case numerical_type::complex_float32:
	case numerical_type::complex_float64:
		return type;

	default: return numerical_type::unknown;
	}
}

numerical_type
promote_types(numerical_type type1, numerical_type type2) noexcept
{
	static const auto lut = compute_numerical_type_promotion_table();

	const auto i = static_cast<int>(type1); 
	const auto j = static_cast<int>(type2);
	const auto n = static_cast<int>(lut.size());

	if (i < 0 || i >= n || j < 0 || j >= n)
	{
		return numerical_type::unknown;
	}

	return lut[i][j];
}

REXLIB_API
const char* to_string(numerical_type type) noexcept
{
	switch (type)
	{
	REX_ENUM_TO_STR_CASE(numerical_type, boolean)
	REX_ENUM_TO_STR_CASE(numerical_type, char8)
	REX_ENUM_TO_STR_CASE(numerical_type, int8)
	REX_ENUM_TO_STR_CASE(numerical_type, uint8)
	REX_ENUM_TO_STR_CASE(numerical_type, int16)
	REX_ENUM_TO_STR_CASE(numerical_type, uint16)
	REX_ENUM_TO_STR_CASE(numerical_type, int32)
	REX_ENUM_TO_STR_CASE(numerical_type, uint32)
	REX_ENUM_TO_STR_CASE(numerical_type, int64)
	REX_ENUM_TO_STR_CASE(numerical_type, uint64)
	REX_ENUM_TO_STR_CASE(numerical_type, float16)
	REX_ENUM_TO_STR_CASE(numerical_type, float32)
	REX_ENUM_TO_STR_CASE(numerical_type, float64)
	REX_ENUM_TO_STR_CASE(numerical_type, complex_float16)
	REX_ENUM_TO_STR_CASE(numerical_type, complex_float32)
	REX_ENUM_TO_STR_CASE(numerical_type, complex_float64)
	default: return "";
	}
}

const char* to_string(numerical_type_category category) noexcept
{
	switch (category)
	{
	REX_ENUM_TO_STR_CASE(numerical_type_category, boolean)
	REX_ENUM_TO_STR_CASE(numerical_type_category, character)
	REX_ENUM_TO_STR_CASE(numerical_type_category, signed_integer)
	REX_ENUM_TO_STR_CASE(numerical_type_category, unsigned_integer)
	REX_ENUM_TO_STR_CASE(numerical_type_category, floating_point)
	REX_ENUM_TO_STR_CASE(numerical_type_category, complex)
	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, numerical_type type)
{
	return os << to_string(type);
}

std::ostream& operator<<(std::ostream& os, numerical_type_category category)
{
	return os << to_string(category);
}

} // namespace rex
