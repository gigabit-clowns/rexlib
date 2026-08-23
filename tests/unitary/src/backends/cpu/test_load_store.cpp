// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/load_store.hpp>

#include <xmipp4/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <type_traits>

using namespace rex;
using namespace rex::cpu;

// Values used with exact equality are exactly representable in IEEE half
// precision, so the float16_t round-trips are lossless.


TEST_CASE( "load returns a non-float16 element unchanged", "[load_store]" )
{
	const double value = GENERATE(0.0, 2.5, -7.0);
	REQUIRE( load(&value) == value );
}

TEST_CASE( "load widens a float16_t element to float", "[load_store]" )
{
	const float value = GENERATE(0.0f, 3.75f, -0.25f);
	const float16_t element(value);

	STATIC_REQUIRE( std::is_same<decltype(load(&element)), float>::value );
	REQUIRE( load(&element) == value );
}

TEST_CASE(
	"load widens a complex<float16_t> element to complex<float>",
	"[load_store]"
)
{
	const std::complex<float16_t> element(float16_t(1.5f), float16_t(-2.25f));

	const std::complex<float> result = load(&element);

	REQUIRE( result.real() == 1.5f );
	REQUIRE( result.imag() == -2.25f );
}


TEST_CASE( "store writes a non-float16 element unchanged", "[load_store]" )
{
	const int value = GENERATE(0, 42, -7);
	int element = 0;

	store(&element, value);

	REQUIRE( element == value );
}

TEST_CASE( "store rounds a float into a float16_t element", "[load_store]" )
{
	const float value = GENERATE(0.0f, 3.75f, -0.25f);
	float16_t element;

	store(&element, value);

	REQUIRE( static_cast<float>(element) == value );
}

TEST_CASE(
	"store rounds a complex<float> into a complex<float16_t> element",
	"[load_store]"
)
{
	std::complex<float16_t> element;

	store(&element, std::complex<float>(1.5f, -2.25f));

	REQUIRE( static_cast<float>(element.real()) == 1.5f );
	REQUIRE( static_cast<float>(element.imag()) == -2.25f );
}


TEST_CASE( "cast converts between scalar types", "[load_store]" )
{
	float widened = 0.0f;
	const int source = 5;
	cast(&widened, &source);
	REQUIRE( widened == 5.0f );

	int truncated = 0;
	const double other = 3.9;
	cast(&truncated, &other);
	REQUIRE( truncated == 3 );
}

TEST_CASE( "cast rounds into a float16_t destination", "[load_store]" )
{
	const float value = GENERATE(0.5f, 3.75f, -8.0f);
	float16_t destination;
	const float source = value;

	cast(&destination, &source);

	REQUIRE( static_cast<float>(destination) == value );
}

TEST_CASE( "cast widens from a float16_t source", "[load_store]" )
{
	double destination = 0.0;
	const float16_t source(2.5f);

	cast(&destination, &source);

	REQUIRE( destination == 2.5 );
}

TEST_CASE(
	"cast copies a float16_t into a float16_t preserving the value",
	"[load_store]"
)
{
	float16_t destination;
	const float16_t source(6.25f);

	cast(&destination, &source);

	REQUIRE( static_cast<float>(destination) == 6.25f );
}

TEST_CASE( "cast converts a scalar into a complex destination", "[load_store]" )
{
	std::complex<float> destination;
	const int source = 4;

	cast(&destination, &source);

	REQUIRE( destination.real() == 4.0f );
	REQUIRE( destination.imag() == 0.0f );
}

TEST_CASE(
	"cast converts a float16_t into a complex<float16_t> destination",
	"[load_store]"
)
{
	std::complex<float16_t> destination;
	const float16_t source(1.5f);

	cast(&destination, &source);

	REQUIRE( static_cast<float>(destination.real()) == 1.5f );
	REQUIRE( static_cast<float>(destination.imag()) == 0.0f );
}

TEST_CASE( "cast converts between complex types", "[load_store]" )
{
	std::complex<double> destination;
	const std::complex<float> source(1.5f, -2.25f);

	cast(&destination, &source);

	REQUIRE( destination.real() == 1.5 );
	REQUIRE( destination.imag() == -2.25 );
}

TEST_CASE(
	"cast widens a complex<float16_t> to a complex<float>",
	"[load_store]"
)
{
	std::complex<float> destination;
	const std::complex<float16_t> source(float16_t(3.5f), float16_t(0.5f));

	cast(&destination, &source);

	REQUIRE( destination.real() == 3.5f );
	REQUIRE( destination.imag() == 0.5f );
}

TEST_CASE(
	"cast rounds a complex<float> into a complex<float16_t>",
	"[load_store]"
)
{
	std::complex<float16_t> destination;
	const std::complex<float> source(2.5f, -4.75f);

	cast(&destination, &source);

	REQUIRE( static_cast<float>(destination.real()) == 2.5f );
	REQUIRE( static_cast<float>(destination.imag()) == -4.75f );
}

TEST_CASE(
	"cast preserves a complex<float16_t> to a complex<float16_t>",
	"[load_store]"
)
{
	std::complex<float16_t> destination;
	const std::complex<float16_t> source(float16_t(1.25f), float16_t(-8.0f));

	cast(&destination, &source);

	REQUIRE( static_cast<float>(destination.real()) == 1.25f );
	REQUIRE( static_cast<float>(destination.imag()) == -8.0f );
}
