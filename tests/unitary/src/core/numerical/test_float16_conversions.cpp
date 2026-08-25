// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <core/numerical/float16_conversions.hpp>

#include <half.hpp>

using namespace rexlib;

TEST_CASE(
	"half_to_float16 and float16_to_half should be inverses",
	"[float16_conversions]"
)
{
	const float value = GENERATE(0.0f, 1.5f, -2.25f, 100.0f);
	const half_float::half original(value);

	const half_float::half result = float16_to_half(half_to_float16(original));

	REQUIRE( static_cast<float>(result) == static_cast<float>(original) );
}

TEST_CASE(
	"float16_to_float should widen like half_float::half",
	"[float16_conversions]"
)
{
	const float value = GENERATE(0.5f, 3.75f, -8.0f);
	const half_float::half original(value);

	const float16_t widened = half_to_float16(original);

	REQUIRE( float16_to_float(widened) == static_cast<float>(original) );
}

TEST_CASE(
	"float_to_float16 should round like half_float::half",
	"[float16_conversions]"
)
{
	// These values are not exactly representable in half precision, so they
	// exercise rounding rather than an exact copy.
	const float value = GENERATE(0.1f, 3.14159f, -1.7f);

	REQUIRE(
		float16_to_float(float_to_float16(value)) ==
		static_cast<float>(half_float::half(value))
	);
}

TEST_CASE(
	"float_to_float16 and float16_to_float should round-trip exact values",
	"[float16_conversions]"
)
{
	const float value = GENERATE(0.0f, 0.5f, -0.25f, 3.75f, 1024.0f);

	REQUIRE( float16_to_float(float_to_float16(value)) == value );
}
