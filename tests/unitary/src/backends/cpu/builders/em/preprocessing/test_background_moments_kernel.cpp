// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/builders/em/preprocessing/background_moments_kernel.hpp>

#include <backends/cpu/builders/em/preprocessing/background_geometry.hpp>
#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinRel;

namespace
{

using moments_type = std::tuple<std::size_t, double, double>;

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
 * @brief Fold part of a row through a kernel as one run.
 */
void fold_run(
	const background_moments_kernel &kernel,
	const std::vector<double> &row,
	moments_type &moments,
	std::size_t first,
	std::size_t last
)
{
	const std::ptrdiff_t coordinate = static_cast<std::ptrdiff_t>(first);
	kernel.combine_run(
		std::make_tuple(
			&std::get<0>(moments),
			&std::get<1>(moments),
			&std::get<2>(moments)
		),
		std::make_tuple(row.data() + first),
		std::make_tuple(contiguous_stride_tag()),
		last - first,
		multidimensional_index(1, 0).rebased(&coordinate)
	);
}

std::array<double, 2> finalize(
	const background_moments_kernel &kernel,
	const moments_type &moments
)
{
	std::array<double, 2> result = {{ 0, 0 }};
	kernel.finalize(
		&result[0],
		&result[1],
		std::get<0>(moments),
		std::get<1>(moments),
		std::get<2>(moments),
		9
	);
	return result;
}

/**
 * @brief Fold a whole row through a kernel as one run.
 *
 * @return std::array<double, 2> The mean and the scale.
 */
std::array<double, 2> fold(
	const background_moments_kernel &kernel,
	const std::vector<double> &row
)
{
	moments_type moments(0, 0, 0);
	fold_run(kernel, row, moments, 0, row.size());
	return finalize(kernel, moments);
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

	moments_type left(0, 0, 0);
	moments_type right(0, 0, 0);
	fold_run(kernel, row, left, 0, 5);
	fold_run(kernel, row, right, 5, 9);
	kernel.merge(
		std::get<0>(left),
		std::get<1>(left),
		std::get<2>(left),
		std::get<0>(right),
		std::get<1>(right),
		std::get<2>(right)
	);

	CHECK( finalize(kernel, left) == fold(kernel, row) );
}

TEST_CASE(
	"background_moments_kernel should seed from one element",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const auto kernel = make_unbounded(geometry);
	const std::array<double, 2> values = {{ 5, not_a_number }};
	const std::array<std::ptrdiff_t, 2> coordinates = {{ 0, 4 }};

	const auto seeded = [&](const double *value, const std::ptrdiff_t *at)
	{
		moments_type result(9, -1, -1);
		kernel.seed(
			std::get<0>(result),
			std::get<1>(result),
			std::get<2>(result),
			value,
			multidimensional_index(1, 0).rebased(at)
		);
		return result;
	};

	CHECK( seeded(&values[0], &coordinates[0]) == moments_type(1, 5, 25) );
	CHECK( seeded(&values[0], &coordinates[1]) == moments_type(0, 0, 0) );
	CHECK( seeded(&values[1], &coordinates[0]) == moments_type(0, 0, 0) );
}

TEST_CASE(
	"background_moments_kernel should fold every strip of a block as its "
	"own run",
	"[background_moments_kernel]"
)
{
	const auto geometry = make_row_geometry();
	const auto kernel = make_unbounded(geometry);

	// Two rows interleaved, so that the strip steps by one between them.
	const std::vector<double> first = { 1, 3, 1, 9, 9, 9, 3, 1, 3 };
	const std::vector<double> second = { 2, 2, 2, 9, 9, 9, 4, 4, 4 };
	std::vector<double> interleaved;
	for (std::size_t i = 0; i < 9; ++i)
	{
		interleaved.push_back(first[i]);
		interleaved.push_back(second[i]);
	}

	std::array<std::size_t, 2> counts = {{ 0, 0 }};
	std::array<double, 2> sums = {{ 0, 0 }};
	std::array<double, 2> squares = {{ 0, 0 }};
	const std::ptrdiff_t coordinate = 0;
	kernel.combine_strip(
		std::make_tuple(counts.data(), sums.data(), squares.data()),
		std::make_tuple(static_cast<const double*>(interleaved.data())),
		std::make_tuple(contiguous_stride_tag()),
		std::make_tuple(std::ptrdiff_t(2)),
		2,
		9,
		multidimensional_index(1, 0).rebased(&coordinate)
	);

	CHECK(
		finalize(kernel, moments_type(counts[0], sums[0], squares[0])) ==
		fold(kernel, first)
	);
	CHECK(
		finalize(kernel, moments_type(counts[1], sums[1], squares[1])) ==
		fold(kernel, second)
	);
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
