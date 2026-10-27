// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/kernels/standard_normal.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE(
	"uniform_open_closed should map random bits into (0, 1]",
	"[standard_normal]"
)
{
	const double step = 1.0 / 9007199254740992.0;

	CHECK( uniform_open_closed(0) == step );
	CHECK( uniform_open_closed(std::uint64_t(1) << 63) == 0.5 + step );
	CHECK(
		uniform_open_closed(std::numeric_limits<std::uint64_t>::max()) ==
		1.0
	);
}

TEST_CASE(
	"standard_normal should take the values of a pair in counter order",
	"[standard_normal]"
)
{
	const std::uint64_t seed = 0x0123456789abcdefu;
	const std::uint64_t block = 0x100000002u;
	const auto pair = standard_normal_pair(seed, block);

	CHECK( standard_normal(seed, 2 * block) == pair[0] );
	CHECK( standard_normal(seed, 2 * block + 1) == pair[1] );
	CHECK( standard_normal(seed, 2 * block + 2) != pair[0] );
	CHECK( standard_normal(seed + 1, 2 * block) != pair[0] );
}

TEST_CASE(
	"standard_normal_pair should take its radius and its angle from the "
	"words of the Philox block",
	"[standard_normal]"
)
{
	// Block 0 under seed 0 is the first known answer of Random123.
	const auto radius_uniform = uniform_open_closed(0xe169c58d6627e8d5u);
	const auto angle_uniform = uniform_open_closed(0x9b00dbd8bc57ac4cu);
	const auto pair = standard_normal_pair(0, 0);

	CHECK_THAT(
		pair[0] * pair[0] + pair[1] * pair[1],
		WithinRel(-2.0 * std::log(radius_uniform), 1e-12)
	);

	const double two_pi = 6.283185307179586476925;
	auto angle = std::atan2(pair[1], pair[0]);
	if (angle < 0)
	{
		angle += two_pi;
	}
	CHECK_THAT( angle, WithinAbs(two_pi * angle_uniform, 1e-12) );
}

TEST_CASE(
	"standard_normal should draw finite values of zero mean and unit "
	"variance",
	"[standard_normal]"
)
{
	const std::size_t count = 200000;
	const std::uint64_t seed = 42;

	double sum = 0;
	double sum_of_squares = 0;
	std::size_t within_one = 0;
	bool finite = true;
	for (std::size_t c = 0; c < count; ++c)
	{
		const auto value = standard_normal(seed, c);
		finite = finite && std::isfinite(value);
		sum += value;
		sum_of_squares += value * value;
		within_one += std::abs(value) <= 1.0 ? 1 : 0;
	}

	const auto mean = sum / count;
	const auto variance = sum_of_squares / count - mean * mean;
	CHECK( finite );
	CHECK_THAT( mean, WithinAbs(0.0, 0.01) );
	CHECK_THAT( variance, WithinAbs(1.0, 0.02) );
	CHECK_THAT(
		static_cast<double>(within_one) / count,
		WithinAbs(0.6827, 0.005)
	);
}
