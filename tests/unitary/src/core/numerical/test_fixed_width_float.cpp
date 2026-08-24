// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/numerical/fixed_width_float.hpp>

using namespace rexlib;

TEST_CASE( "float16_t default constructor should produce zero", "[float16_t]" )
{
	float16_t x;
	REQUIRE( static_cast<float>(x) == 0.0f );
}

TEST_CASE( "float16_t should be constructible from int", "[float16_t]" )
{
	REQUIRE( static_cast<float>(float16_t(0)) == 0.0f );
	REQUIRE( static_cast<float>(float16_t(42)) == 42.0f );
	REQUIRE( static_cast<float>(float16_t(-7)) == -7.0f );
	REQUIRE( static_cast<float>(float16_t(1024)) == 1024.0f );
}

TEST_CASE( "float16_t should be constructible from float", "[float16_t]" )
{
	REQUIRE( static_cast<float>(float16_t(0.5f)) == 0.5f );
	REQUIRE( static_cast<float>(float16_t(-0.25f)) == -0.25f );
	REQUIRE( static_cast<float>(float16_t(3.75f)) == 3.75f );
}

TEST_CASE( "float16_t should be constructible from double", "[float16_t]" )
{
	REQUIRE( static_cast<float>(float16_t(0.5)) == 0.5f );
	REQUIRE( static_cast<float>(float16_t(-128.0)) == -128.0f );
	REQUIRE( static_cast<float>(float16_t(6.25)) == 6.25f );
}

TEST_CASE(
	"float16_t copy constructor should preserve the value",
	"[float16_t]"
)
{
	float16_t x(2.5f);
	float16_t y(x);
	REQUIRE( static_cast<float>(y) == 2.5f );
	REQUIRE( static_cast<float>(x) == 2.5f );
}

TEST_CASE(
	"float16_t move constructor should preserve the value",
	"[float16_t]"
)
{
	float16_t x(-9.0f);
	float16_t y(std::move(x));
	REQUIRE( static_cast<float>(y) == -9.0f );
}

TEST_CASE(
	"float16_t copy assignment should preserve the value",
	"[float16_t]"
)
{
	float16_t x(12.5f);
	float16_t y;
	y = x;
	REQUIRE( static_cast<float>(y) == 12.5f );
	REQUIRE( static_cast<float>(x) == 12.5f );
}

TEST_CASE(
	"float16_t move assignment should preserve the value",
	"[float16_t]"
)
{
	float16_t x(0.125f);
	float16_t y;
	y = std::move(x);
	REQUIRE( static_cast<float>(y) == 0.125f );
}
