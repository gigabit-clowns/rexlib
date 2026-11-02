// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <backends/cpu/kernels/row_interval.hpp>

#include <cstddef>
#include <limits>
#include <random>

using namespace rexlib::cpu;

namespace
{

bool within(
	const volume_frequency &base,
	const volume_frequency &step,
	const frequency_bounds &bounds,
	std::ptrdiff_t k
)
{
	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		const auto value = base[axis] + static_cast<double>(k) * step[axis];
		if (value < bounds.get_lower(axis) || value > bounds.get_upper(axis))
		{
			return false;
		}
	}
	return true;
}

} // anonymous namespace

TEST_CASE(
	"row_interval should report its ends and whether it is empty",
	"[row_interval]"
)
{
	const row_interval filled(2, 5);
	const row_interval empty(5, 5);

	CHECK( filled.get_begin() == 2 );
	CHECK( filled.get_end() == 5 );
	CHECK_FALSE( filled.is_empty() );
	CHECK( empty.is_empty() );
	CHECK( row_interval(6, 5).is_empty() );
}

TEST_CASE(
	"clip_row should keep every sample within the bounds and at most two "
	"more",
	"[row_interval]"
)
{
	const auto seed = GENERATE(range(0, 64));
	std::mt19937 engine(static_cast<std::mt19937::result_type>(seed));
	std::uniform_real_distribution<double> coordinate(-20.0, 20.0);
	std::uniform_real_distribution<double> slope(-3.0, 3.0);

	const volume_frequency base = {{
		coordinate(engine), coordinate(engine), coordinate(engine)
	}};
	const volume_frequency step = {{
		slope(engine), slope(engine), slope(engine)
	}};
	volume_frequency lower{};
	volume_frequency upper{};
	for (std::size_t axis = 0; axis < 3; ++axis)
	{
		const auto a = coordinate(engine);
		const auto b = coordinate(engine);
		lower[axis] = a < b ? a : b;
		upper[axis] = a < b ? b : a;
	}
	const frequency_bounds bounds(lower, upper);

	const auto interval = clip_row(base, step, bounds, 0, 16);

	for (std::ptrdiff_t k = 0; k <= 16; ++k)
	{
		if (within(base, step, bounds, k))
		{
			CHECK( interval.get_begin() <= k );
			CHECK( k < interval.get_end() );
		}
	}
	std::ptrdiff_t within_count = 0;
	for (std::ptrdiff_t k = 0; k <= 16; ++k)
	{
		within_count += within(base, step, bounds, k) ? 1 : 0;
	}
	CHECK( interval.get_end() - interval.get_begin() <= within_count + 2 );
}

TEST_CASE(
	"clip_row should keep a row that does not move along an axis when its "
	"base lies within the bounds, and drop it otherwise",
	"[row_interval]"
)
{
	const frequency_bounds bounds({{ 0, 0, 0 }}, {{ 10, 10, 10 }});
	const volume_frequency step = {{ 1, 0, 0 }};

	const auto kept = clip_row({{ 0, 5, 5 }}, step, bounds, 0, 20);
	CHECK( kept.get_begin() == 0 );
	CHECK( kept.get_end() == 11 );

	const auto dropped = clip_row({{ 0, 5, 11 }}, step, bounds, 0, 20);
	CHECK( dropped.is_empty() );
}

TEST_CASE(
	"clip_row should give an empty range for a frame that is not finite",
	"[row_interval]"
)
{
	const auto not_a_number = std::numeric_limits<double>::quiet_NaN();
	const frequency_bounds bounds({{ 0, 0, 0 }}, {{ 10, 10, 10 }});

	CHECK(
		clip_row({{ not_a_number, 0, 0 }}, {{ 1, 0, 0 }}, bounds, 0, 8)
			.is_empty()
	);
	CHECK(
		clip_row({{ 0, 0, 0 }}, {{ not_a_number, 0, 0 }}, bounds, 0, 8)
			.is_empty()
	);
}
