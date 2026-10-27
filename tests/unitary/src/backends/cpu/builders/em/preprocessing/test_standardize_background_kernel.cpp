// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/em/preprocessing/standardize_background_kernel.hpp>

#include <backends/cpu/kernels/standard_normal.hpp>
#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/linear_index_run.hpp>
#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

/**
 * @brief A row of nine elements whose background is indices 0 to 2 and 6
 * to 8, kept within one standard deviation of the mean.
 */
standardize_background_kernel make_kernel()
{
	const std::array<std::size_t, 1> extents = {{ 9 }};
	return standardize_background_kernel(
		background_geometry(make_span(extents), 1),
		-1,
		1
	);
}

std::array<double, 2> fold(
	const background_moments_kernel &kernel,
	const std::vector<double> &row
)
{
	std::array<double, 3> moments = {{ 0, 0, 0 }};
	for (std::size_t i = 0; i < row.size(); ++i)
	{
		const std::ptrdiff_t coordinate = static_cast<std::ptrdiff_t>(i);
		const auto index = multidimensional_index(1, 1).rebased(&coordinate);
		kernel.combine(moments[0], moments[1], moments[2], &row[i], index);
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
	"standardize_background_kernel should keep every finite background "
	"element in the first pass, and bound them in the second",
	"[standardize_background_kernel]"
)
{
	const auto kernel = make_kernel();
	const std::vector<double> row = { 1, 3, 1, 0, 0, 0, 3, 1, 23 };

	// The first pass takes 23 in: a mean of 5.33 and a deviation of 7.95.
	const auto first = fold(kernel.make_reduction(0, {{ 0, 1 }}), row);
	CHECK( first[0] > 5 );

	// Standardized with those, 23 lies above 1 and the rest within.
	const auto second = fold(kernel.make_reduction(1, first), row);
	CHECK( second[0] == 1.8 );
}

TEST_CASE(
	"standardize_background_kernel should make the map of an image from its "
	"statistics, its seed and its counter",
	"[standardize_background_kernel]"
)
{
	const auto kernel = make_kernel();
	const double value = 3;
	const std::uint64_t seed = 12;

	const auto map = kernel.make_map(
		{{ 1, 0.5 }},
		std::make_tuple(&value, &seed),
		40
	);

	const std::vector<double> values = { 3, 9 };
	std::vector<double> result(2, -1);
	map(
		std::make_tuple(result.data(), values.data()),
		std::make_tuple(contiguous_stride_tag(), contiguous_stride_tag()),
		2,
		linear_index_run<contiguous_stride_tag>(2, contiguous_stride_tag())
	);

	CHECK( result[0] == 1 );
	CHECK( result[1] == standard_normal(seed, 43) );
}
