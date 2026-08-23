// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/numerical/scalar_value.hpp>
#include <rex/core/numerical/numerical_type_traits.hpp>

#include <complex>
#include <type_traits>
#include <typeinfo>

using namespace rex;

TEST_CASE(
	"scalar_value::get_data_type reports the numerical type of the stored "
	"value",
	"[scalar_value]"
)
{
	CHECK( scalar_value(true).get_data_type() == numerical_type::boolean );
	CHECK( scalar_value(std::int32_t(0)).get_data_type() == numerical_type::int32 );
	CHECK( scalar_value(float64_t(0)).get_data_type() == numerical_type::float64 );
	CHECK(
		scalar_value(std::complex<float32_t>()).get_data_type() ==
		numerical_type::complex_float32
	);
}

TEST_CASE(
	"scalar_value::get returns the stored value when the type matches",
	"[scalar_value]"
)
{
	const scalar_value value(std::int32_t(42));

	CHECK( value.get<std::int32_t>() == 42 );
}

TEST_CASE(
	"scalar_value::get throws bad_scalar_value_access when the type does not "
	"match",
	"[scalar_value]"
)
{
	const scalar_value value(std::int32_t(42));

	CHECK_THROWS_AS( value.get<float64_t>(), bad_scalar_value_access );
}

TEST_CASE(
	"scalar_value::get_if returns a pointer to the stored value when the type "
	"matches",
	"[scalar_value]"
)
{
	const scalar_value value(float64_t(2.5));

	const float64_t *match = value.get_if<float64_t>();
	REQUIRE( match != nullptr );
	CHECK( *match == 2.5 );
}

TEST_CASE(
	"scalar_value::get_if returns nullptr when the type does not match",
	"[scalar_value]"
)
{
	const scalar_value value(float64_t(2.5));

	CHECK( value.get_if<std::int32_t>() == nullptr );
}

TEST_CASE(
	"scalar_value stores a copy that is independent from the source value",
	"[scalar_value]"
)
{
	std::int32_t source = 7;
	const scalar_value value(source);

	// Mutating the source must not affect the stored copy.
	source = 9;

	CHECK( value.get<std::int32_t>() == 7 );
	CHECK( value.get_data_type() == numerical_type::int32 );
}

TEST_CASE(
	"copying a scalar_value preserves the stored value",
	"[scalar_value]"
)
{
	const scalar_value original(std::complex<float64_t>(1.0, -2.0));

	const scalar_value copy(original);

	CHECK( copy.get_data_type() == numerical_type::complex_float64 );
	CHECK(
		copy.get<std::complex<float64_t>>() ==
		std::complex<float64_t>(1.0, -2.0)
	);
}

TEST_CASE(
	"visit dispatches to the stored concrete type",
	"[scalar_value]"
)
{
	// The visitor is instantiated for every native type, so it must be valid
	// for all of them, just like a std::visit visitor over a std::variant.
	const auto get_type = [] (const auto &x)
	{
		return numerical_type_of<std::decay_t<decltype(x)>>::value;
	};

	CHECK( visit(get_type, scalar_value(std::int32_t(21))) == numerical_type::int32 );
	CHECK( visit(get_type, scalar_value(float64_t(1.0))) == numerical_type::float64 );
	CHECK(
		visit(get_type, scalar_value(std::complex<float32_t>())) ==
		numerical_type::complex_float32
	);
}

TEST_CASE(
	"scalar_value_cast returns the stored value when the type matches",
	"[scalar_value]"
)
{
	const scalar_value value(std::int32_t(42));

	CHECK( scalar_value_cast<std::int32_t>(value) == 42 );
}

TEST_CASE(
	"scalar_value_cast numerically converts between compatible types",
	"[scalar_value]"
)
{
	// Widening and narrowing between real types.
	CHECK( scalar_value_cast<float64_t>(scalar_value(std::int32_t(7))) == 7.0 );
	CHECK( scalar_value_cast<std::int32_t>(scalar_value(float64_t(2.5))) == 2 );

	// A real value promotes into the real part of a complex destination.
	CHECK(
		scalar_value_cast<std::complex<float32_t>>(scalar_value(float32_t(3.0f))) ==
		std::complex<float32_t>(3.0f, 0.0f)
	);

	// Conversion between complex types.
	CHECK(
		scalar_value_cast<std::complex<float32_t>>(
			scalar_value(std::complex<float64_t>(1.0, -2.0))
		) == std::complex<float32_t>(1.0f, -2.0f)
	);
}

TEST_CASE(
	"scalar_value_cast throws bad_cast when the stored value can not be "
	"converted",
	"[scalar_value]"
)
{
	// A complex value can not be narrowed into a real destination.
	const scalar_value value(std::complex<float32_t>(1.0f, 2.0f));

	CHECK_THROWS_AS( scalar_value_cast<float32_t>(value), std::bad_cast );
}
