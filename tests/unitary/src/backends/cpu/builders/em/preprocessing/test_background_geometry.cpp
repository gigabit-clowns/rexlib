// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/builders/em/preprocessing/background_geometry.hpp>

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
#include <limits>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

bool is_background(
	const background_geometry &geometry,
	std::vector<std::ptrdiff_t> coordinates
)
{
	const auto rank = coordinates.size();
	return geometry.is_background(
		multidimensional_index(rank, rank).rebased(coordinates.data())
	);
}

std::vector<std::ptrdiff_t> centre_of(const background_geometry &geometry)
{
	const auto centre = geometry.get_centre();
	return std::vector<std::ptrdiff_t>(centre.begin(), centre.end());
}

} // anonymous namespace

TEST_CASE(
	"background_geometry should centre an image at the index of half its "
	"extent, rounded down",
	"[background_geometry]"
)
{
	const std::array<std::size_t, 3> extents = {{ 4, 5, 1 }};
	const background_geometry geometry(make_span(extents), 1);

	CHECK( centre_of(geometry) == std::vector<std::ptrdiff_t>{ 2, 2, 0 } );
	CHECK( geometry.get_squared_radius() == 1 );
}

TEST_CASE(
	"background_geometry should leave out the elements at the radius or "
	"nearer",
	"[background_geometry]"
)
{
	const std::array<std::size_t, 2> extents = {{ 5, 5 }};
	const background_geometry geometry(make_span(extents), 2);

	CHECK_FALSE( is_background(geometry, { 2, 2 }) );
	CHECK_FALSE( is_background(geometry, { 0, 2 }) );
	CHECK_FALSE( is_background(geometry, { 2, 4 }) );
	CHECK( is_background(geometry, { 0, 1 }) );
	CHECK( is_background(geometry, { 0, 0 }) );
}

TEST_CASE(
	"background_geometry should measure distances alike along every axis "
	"of an image that is not square",
	"[background_geometry]"
)
{
	const std::array<std::size_t, 2> extents = {{ 4, 8 }};
	const background_geometry geometry(make_span(extents), 2.5);

	CHECK_FALSE( is_background(geometry, { 2, 6 }) );
	CHECK( is_background(geometry, { 2, 7 }) );
	CHECK( is_background(geometry, { 0, 6 }) );
	CHECK_FALSE( is_background(geometry, { 0, 4 }) );
}

TEST_CASE(
	"background_geometry should leave out only the centre at a radius of 0",
	"[background_geometry]"
)
{
	const std::array<std::size_t, 2> extents = {{ 3, 3 }};
	const background_geometry geometry(make_span(extents), 0);

	CHECK_FALSE( is_background(geometry, { 1, 1 }) );
	CHECK( is_background(geometry, { 1, 2 }) );
}

TEST_CASE(
	"background_geometry should find exactly the elements of a run that are "
	"not background",
	"[background_geometry]"
)
{
	const std::array<std::size_t, 2> extents = {{ 7, 10 }};
	const auto radius = GENERATE(
		0.0,
		1.0,
		2.5,
		3.0,
		4.2,
		100.0,
		std::numeric_limits<double>::infinity()
	);
	const background_geometry geometry(make_span(extents), radius);

	for (std::size_t axis = 0; axis < 2; ++axis)
	{
		const auto other = 1 - axis;
		for (std::size_t fixed = 0; fixed < extents[other]; ++fixed)
		{
			// Runs starting part way along the axis too.
			for (std::size_t start = 0; start < 3; ++start)
			{
				std::array<std::ptrdiff_t, 2> coordinates = {{ 0, 0 }};
				coordinates[axis] = static_cast<std::ptrdiff_t>(start);
				coordinates[other] = static_cast<std::ptrdiff_t>(fixed);
				const auto index = multidimensional_index(2, axis)
					.rebased(coordinates.data());
				const auto count = extents[axis] - start;

				const auto range = geometry.find_excluded_range(index, count);
				REQUIRE( range.first <= range.second );
				for (std::size_t e = 0; e < count; ++e)
				{
					INFO(
						"radius " << radius << ", axis " << axis << ", fixed "
						<< fixed << ", start " << start << ", element " << e
					);
					const auto excluded = e >= range.first && e < range.second;
					CHECK(
						excluded ==
						!geometry.is_background(index.advanced(e))
					);
				}
			}
		}
	}
}
