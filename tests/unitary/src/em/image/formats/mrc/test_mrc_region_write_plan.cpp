// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_region_write_plan.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

// Two orders that disagree about which axis should be innermost, so that
// whichever operand the layout was built for is the one walked contiguously.
const std::vector<std::size_t> region_extents = {4, 8};
const std::vector<std::ptrdiff_t> row_major = {8, 1};
const std::vector<std::ptrdiff_t> column_major = {1, 4};

image_transfer_plan one_region()
{
	image_transfer_plan regions(make_span(region_extents), 2, 2);
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	return regions;
}

} // anonymous namespace

TEST_CASE( "a write orders its axes for the file it writes into",
	"[mrc_region_write_plan]" )
{
	const auto regions = one_region();
	const mrc_region_write_plan plan(
		regions,
		make_span(region_extents), make_span(column_major),
		make_span(region_extents), make_span(row_major),
		0
	);

	SECTION( "the file is the operand named first" )
	{
		REQUIRE( plan.get_layout().get_strides(0)[0] == 1 );
		REQUIRE( plan.get_layout().get_strides(1)[0] != 1 );
	}

	SECTION( "the regions are resolved along with it" )
	{
		REQUIRE( plan.get_offsets().get_region_count() == 1 );
	}
}

TEST_CASE( "a batch that does not fit is refused by a write",
	"[mrc_region_write_plan]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::ptrdiff_t> strides = {2, 1};

	image_transfer_plan regions(make_span(extents), 2, 2);
	regions.add(make_span(std::vector<std::size_t>{1, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	REQUIRE_THROWS_AS(
		mrc_region_write_plan(
			regions,
			make_span(extents), make_span(strides),
			make_span(extents), make_span(strides),
			0
		),
		std::out_of_range
	);
}
