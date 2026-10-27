// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/builders/em/preprocessing/background_moments_kernel.hpp>

#include <backends/cpu/builders/em/preprocessing/background_geometry.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinRel;

namespace
{

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();

/**
 * @brief A row of nine elements whose centre, at index 4, lies within a
 * radius of 1 together with indices 3 and 5.
 */
background_geometry make_row_geometry()
{
	const std::array<std::size_t, 1> extents = {{ 9 }};
	return background_geometry(make_span(extents), 1);
}

background_moments_kernel make_unbounded(const background_geometry &geometry)
{
	return background_moments_kernel(geometry, 0, 1, -infinity, infinity);
}

/**
 * @brief Fold a row through a kernel, one element at a time.
 *
 * @return std::array<double, 2> The mean and the scale.
 */
std::array<double, 2> fold(
	const background_moments_kernel &kernel,
	const std::vector<double> &row,
	std::size_t first = 0,
	std::size_t last = 9
)
{
	std::array<double, 3> moments = {{ 0, 0, 0 }};
	for (std::size_t i = first; i < last; ++i)
	{
		const std::ptrdiff_t coordinate = static_cast<std::ptrdiff_t>(i);
		const auto index = multidimensional_index(1, 1).rebased(&coordinate);
		if (i == first)
		{
			kernel.seed(moments[0], moments[1], moments[2], &row[i], index);
		}
		else
		{
			kernel.combine(moments[0], moments[1], moments[2], &row[i], index);
		}
	}

	std::array<double, 2> result = {{ 0, 0 }};
	kernel.finalize(
		&result[0],
		&result[1],
		moments[0],
		moments[1],
		moments[2],
		row.size()
	);
	return result;
}

} // anonymous namespace

TEST_CASE(
	"background_moments_kernel should finalize to the mean and the inverse "
	"standard deviation of the background",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const std::vector<double> row = { 1, 3, 1, 99, 99, 99, 3, 1, 3 };

	const auto statistics = fold(make_unbounded(geometry), row);

	CHECK( statistics[0] == 2 );
	CHECK( statistics[1] == 1 );
}

TEST_CASE(
	"background_moments_kernel should skip every element that is not finite",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const std::vector<double> row = {
		1, not_a_number, 3, 0, 0, 0, infinity, 1, -infinity
	};

	const auto statistics = fold(make_unbounded(geometry), row);

	// The background left is 1, 3 and 1.
	CHECK_THAT( statistics[0], WithinRel(5.0 / 3.0, 1e-12) );
	CHECK_THAT( statistics[1], WithinRel(1.0 / std::sqrt(8.0 / 9.0), 1e-12) );
}

TEST_CASE(
	"background_moments_kernel should skip the elements whose standardized "
	"value lies outside the bounds",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const std::vector<double> row = { 1, 3, 1, 0, 0, 0, 3, 50, 3 };

	// Standardized with a mean of 2 and a scale of 1, 50 lies at 48. The
	// background left is 1, 3, 1, 3 and 3.
	const background_moments_kernel kernel(geometry, 2, 1, -3, 3);

	const auto statistics = fold(kernel, row);

	CHECK_THAT( statistics[0], WithinRel(2.2, 1e-12) );
	CHECK_THAT( statistics[1], WithinRel(1.0 / std::sqrt(0.96), 1e-12) );
}

TEST_CASE(
	"background_moments_kernel should merge partial folds into the fold of "
	"their union",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const auto kernel = make_unbounded(geometry);
	const std::vector<double> row = { 1, 3, 1, 0, 0, 0, 3, 1, 3 };

	std::array<double, 3> left = {{ 0, 0, 0 }};
	std::array<double, 3> right = {{ 0, 0, 0 }};
	for (std::size_t i = 0; i < 9; ++i)
	{
		const std::ptrdiff_t coordinate = static_cast<std::ptrdiff_t>(i);
		const auto index = multidimensional_index(1, 1).rebased(&coordinate);
		auto &part = i < 5 ? left : right;
		if (i == 0 || i == 5)
		{
			kernel.seed(part[0], part[1], part[2], &row[i], index);
		}
		else
		{
			kernel.combine(part[0], part[1], part[2], &row[i], index);
		}
	}
	kernel.merge(left[0], left[1], left[2], right[0], right[1], right[2]);

	std::array<double, 2> merged = {{ 0, 0 }};
	kernel.finalize(&merged[0], &merged[1], left[0], left[1], left[2], 9);

	CHECK( merged == fold(kernel, row) );
}

TEST_CASE(
	"background_moments_kernel should give a mean of 0 and a scale of 1 "
	"without a valid element",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const std::vector<double> row(9, not_a_number);

	CHECK(
		fold(make_unbounded(geometry), row) ==
		std::array<double, 2>{{ 0, 1 }}
	);
}

TEST_CASE(
	"background_moments_kernel should give a scale of 1 to a constant "
	"background",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const std::vector<double> row(9, 0.1f);

	const auto statistics = fold(make_unbounded(geometry), row);

	CHECK_THAT( statistics[0], WithinRel(static_cast<double>(0.1f), 1e-12) );
	CHECK( statistics[1] == 1 );
}
