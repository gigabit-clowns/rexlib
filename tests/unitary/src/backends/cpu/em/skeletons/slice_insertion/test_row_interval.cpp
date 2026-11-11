// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/row_interval.hpp>

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

TEST_CASE(
	"bound_rows should keep every row with a sample within the bounds",
	"[row_interval]"
)
{
	const auto seed = GENERATE(range(0, 64));
	std::mt19937 engine(static_cast<std::mt19937::result_type>(seed + 100));
	std::uniform_real_distribution<double> coordinate(-20.0, 20.0);
	std::uniform_real_distribution<double> slope(-3.0, 3.0);

	const volume_frequency x_step = {{
		slope(engine), slope(engine), slope(engine)
	}};
	const volume_frequency y_step = {{
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

	const auto rows = bound_rows(x_step, y_step, bounds, 0, 12, -12, 12);

	for (std::ptrdiff_t k_y = -12; k_y <= 12; ++k_y)
	{
		const volume_frequency base = {{
			static_cast<double>(k_y) * y_step[0],
			static_cast<double>(k_y) * y_step[1],
			static_cast<double>(k_y) * y_step[2]
		}};
		for (std::ptrdiff_t k_x = 0; k_x <= 12; ++k_x)
		{
			if (within(base, x_step, bounds, k_x))
			{
				CHECK( rows.get_begin() <= k_y );
				CHECK( k_y < rows.get_end() );
			}
		}
	}
	CHECK( rows.get_begin() >= -12 );
	CHECK( rows.get_end() <= 13 );
}

TEST_CASE(
	"bound_rows should give no row for a box the slice cannot reach",
	"[row_interval]"
)
{
	const frequency_bounds bounds({{ 0, 0, 5 }}, {{ 10, 10, 6 }});

	CHECK(
		bound_rows({{ 1, 0, 0 }}, {{ 0, 1, 0 }}, bounds, 0, 8, -8, 8)
			.is_empty()
	);
	CHECK(
		bound_rows(
			{{ 1, 0, 0 }},
			{{ std::numeric_limits<double>::quiet_NaN(), 1, 0 }},
			bounds, 0, 8, -8, 8
		).is_empty()
	);
}
