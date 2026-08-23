// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/ordering.hpp>

#include <cmath>
#include <cstdint>
#include <limits>

using namespace rex;
using namespace rex::cpu;

namespace
{

const float not_a_number = std::numeric_limits<float>::quiet_NaN();

} // anonymous namespace

TEST_CASE(
	"maximum_of and minimum_of order integral elements",
	"[ordering]"
)
{
	CHECK( maximum_of(2, 5) == 5 );
	CHECK( maximum_of(5, 2) == 5 );
	CHECK( minimum_of(2, 5) == 2 );
	CHECK( minimum_of(5, 2) == 2 );
}

TEST_CASE(
	"maximum_of and minimum_of propagate a not-a-number from either side",
	"[ordering]"
)
{
	CHECK( std::isnan(maximum_of(not_a_number, 1.0F)) );
	CHECK( std::isnan(maximum_of(1.0F, not_a_number)) );
	CHECK( std::isnan(minimum_of(not_a_number, 1.0F)) );
	CHECK( std::isnan(minimum_of(1.0F, not_a_number)) );
}

TEST_CASE(
	"maximum_of and minimum_of reduce to disjunction and conjunction "
	"for booleans",
	"[ordering]"
)
{
	CHECK( maximum_of(true, false) );
	CHECK_FALSE( maximum_of(false, false) );
	CHECK( minimum_of(true, true) );
	CHECK_FALSE( minimum_of(true, false) );
}

TEST_CASE(
	"exceeds and precedes compare integral elements strictly",
	"[ordering]"
)
{
	CHECK( exceeds(5, 2) );
	CHECK_FALSE( exceeds(2, 5) );
	CHECK( precedes(2, 5) );
	CHECK_FALSE( precedes(5, 2) );

	// Strict, so an equal element does not displace the one already held and
	// a tie keeps the first place it was seen.
	CHECK_FALSE( exceeds(3, 3) );
	CHECK_FALSE( precedes(3, 3) );
}

TEST_CASE(
	"exceeds and precedes compare signed and unsigned integers",
	"[ordering]"
)
{
	CHECK( exceeds(std::int64_t(-1), std::int64_t(-7)) );
	CHECK( precedes(std::int64_t(-7), std::int64_t(-1)) );
	CHECK( exceeds(std::uint8_t(200), std::uint8_t(3)) );
	CHECK( precedes(std::uint8_t(3), std::uint8_t(200)) );
}

TEST_CASE(
	"exceeds and precedes let a not-a-number displace a number",
	"[ordering]"
)
{
	CHECK( exceeds(not_a_number, 1.0F) );
	CHECK( precedes(not_a_number, 1.0F) );
}

TEST_CASE(
	"exceeds and precedes keep the first not-a-number",
	"[ordering]"
)
{
	// Nothing displaces one already held, so the first one encountered is
	// the one reported.
	CHECK_FALSE( exceeds(1.0F, not_a_number) );
	CHECK_FALSE( precedes(1.0F, not_a_number) );
	CHECK_FALSE( exceeds(not_a_number, not_a_number) );
	CHECK_FALSE( precedes(not_a_number, not_a_number) );
}

TEST_CASE(
	"exceeds and precedes order booleans without comparing them",
	"[ordering]"
)
{
	CHECK( exceeds(true, false) );
	CHECK_FALSE( exceeds(false, true) );
	CHECK_FALSE( exceeds(true, true) );
	CHECK_FALSE( exceeds(false, false) );

	CHECK( precedes(false, true) );
	CHECK_FALSE( precedes(true, false) );
	CHECK_FALSE( precedes(true, true) );
	CHECK_FALSE( precedes(false, false) );
}

TEST_CASE(
	"exceeds and precedes agree with maximum_of and minimum_of",
	"[ordering]"
)
{
	// The predicate answers with which of the two won, where the pair of
	// functions above answer with the winner itself.
	const double values[] = { -2.5, 0.0, 1.0, 7.25 };

	for (const double x : values)
	{
		for (const double y : values)
		{
			CHECK( maximum_of(x, y) == (exceeds(x, y) ? x : y) );
			CHECK( minimum_of(x, y) == (precedes(x, y) ? x : y) );
		}
	}
}
