// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/numerical/numerical_cast.hpp>

#include <xmipp4/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <type_traits>

using namespace rex;

TEST_CASE(
	"numerical_cast returns the destination type",
	"[numerical_cast]"
)
{
	STATIC_REQUIRE(
		std::is_same<decltype(numerical_cast<int>(1.0)), int>::value
	);
	STATIC_REQUIRE(
		std::is_same<decltype(numerical_cast<float16_t>(1.0)), float16_t>::value
	);
	STATIC_REQUIRE(
		std::is_same<
			decltype(numerical_cast<std::complex<double>>(std::complex<float>())),
			std::complex<double>
		>::value
	);
}

TEST_CASE(
	"numerical_cast behaves like static_cast for ordinary arithmetic types",
	"[numerical_cast]"
)
{
	CHECK( numerical_cast<int>(3.9) == 3 );
	CHECK( numerical_cast<double>(7) == 7.0 );
	CHECK( numerical_cast<float>(2.5) == 2.5f );
	CHECK( numerical_cast<unsigned>(5) == 5u );
}

TEST_CASE(
	"numerical_cast converts to float16_t through float",
	"[numerical_cast]"
)
{
	// Small integers and halves are represented exactly by float16_t, so the
	// round trip is lossless and comparable.
	CHECK( static_cast<float>(numerical_cast<float16_t>(2.0)) == 2.0f );
	CHECK( static_cast<float>(numerical_cast<float16_t>(-3.5)) == -3.5f );
	CHECK( static_cast<float>(numerical_cast<float16_t>(7)) == 7.0f );
	CHECK( static_cast<float>(numerical_cast<float16_t>(0.25f)) == 0.25f );
}

TEST_CASE(
	"numerical_cast converts from float16_t through float",
	"[numerical_cast]"
)
{
	const float16_t value(6.5f);
	CHECK( numerical_cast<double>(value) == 6.5 );
	CHECK( numerical_cast<float>(value) == 6.5f );
	CHECK( numerical_cast<int>(value) == 6 );
}

TEST_CASE(
	"numerical_cast round-trips float16_t values",
	"[numerical_cast]"
)
{
	const float16_t original(-12.0f);
	const auto as_double = numerical_cast<double>(original);
	const auto back = numerical_cast<float16_t>(as_double);
	CHECK( static_cast<float>(back) == -12.0f );

	// float16_t to float16_t is an identity-like copy.
	const auto copy = numerical_cast<float16_t>(original);
	CHECK( static_cast<float>(copy) == static_cast<float>(original) );
}

TEST_CASE(
	"numerical_cast converts between complex types element-wise",
	"[numerical_cast]"
)
{
	const std::complex<float> src(1.5f, -2.5f);
	const auto dst = numerical_cast<std::complex<double>>(src);
	CHECK( dst.real() == 1.5 );
	CHECK( dst.imag() == -2.5 );
}

TEST_CASE(
	"numerical_cast handles complex values holding float16_t components",
	"[numerical_cast]"
)
{
	SECTION("towards complex<float16_t>")
	{
		const std::complex<double> src(4.0, -8.0);
		const auto dst = numerical_cast<std::complex<float16_t>>(src);
		CHECK( static_cast<float>(dst.real()) == 4.0f );
		CHECK( static_cast<float>(dst.imag()) == -8.0f );
	}

	SECTION("away from complex<float16_t>")
	{
		const std::complex<float16_t> src(float16_t(3.0f), float16_t(5.0f));
		const auto dst = numerical_cast<std::complex<double>>(src);
		CHECK( dst.real() == 3.0 );
		CHECK( dst.imag() == 5.0 );
	}
}

TEST_CASE(
	"numerical_cast promotes a scalar into a complex value",
	"[numerical_cast]"
)
{
	SECTION("ordinary scalar")
	{
		const auto dst = numerical_cast<std::complex<double>>(9.0);
		CHECK( dst.real() == 9.0 );
		CHECK( dst.imag() == 0.0 );
	}

	SECTION("float16_t scalar into complex<double>")
	{
		const auto dst = numerical_cast<std::complex<double>>(float16_t(2.5f));
		CHECK( dst.real() == 2.5 );
		CHECK( dst.imag() == 0.0 );
	}

	SECTION("scalar into complex<float16_t>")
	{
		const auto dst = numerical_cast<std::complex<float16_t>>(1.5);
		CHECK( static_cast<float>(dst.real()) == 1.5f );
		CHECK( static_cast<float>(dst.imag()) == 0.0f );
	}
}
