// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/em/preprocessing/background_geometry.hpp>

#include <backends/cpu/loops/multidimensional_index.hpp>

#include <rexlib/core/span.hpp>

#include <array>
#include <cstddef>
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
