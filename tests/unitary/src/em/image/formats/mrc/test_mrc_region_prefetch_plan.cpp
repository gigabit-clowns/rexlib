// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_region_prefetch_plan.hpp>

#include <em/image/formats/mrc/mrc_geometry.hpp>
#include <em/image/formats/mrc/mrc_header.hpp>
#include <em/image/formats/mrc/mrc_mode.hpp>

#include <rexlib/core/system/host.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

// A stack of five 3x4 planes of float32, so one plane is 48 bytes.
REXLIB_CONST_CONSTEXPR std::size_t plane_bytes = 3 * 4 * sizeof(float);

// Small enough that the planes of the fixture do not all land in one page,
// which the page of a machine would make them do.
REXLIB_CONST_CONSTEXPR std::size_t test_page = 16;

mrc_geometry make_stack_geometry()
{
	mrc_header header;
	header.set_column_count(4);
	header.set_row_count(3);
	header.set_section_count(5);
	header.set_section_sampling(1);
	header.set_space_group(0);
	header.set_mode(mrc_mode::float32);

	return mrc_geometry(header);
}

const std::vector<std::size_t> plane = {3, 4};

void add_plane(image_transfer_plan &regions, std::size_t section)
{
	const std::size_t file_offset[3] = {section, 0, 0};
	const std::size_t array_offset[3] = {0, 0, 0};
	regions.add(make_span(file_offset, 3), make_span(array_offset, 3));
}

// Where the planes named start in the file, in elements and ascending, which
// is how mrc_region_offsets holds them.
std::vector<std::ptrdiff_t> plane_offsets(
	const std::vector<std::size_t> &sections
)
{
	std::vector<std::ptrdiff_t> offsets;
	for (const auto section : sections)
	{
		offsets.push_back(static_cast<std::ptrdiff_t>(section * 3 * 4));
	}

	return offsets;
}

mrc_prefetch_policy make_policy(
	std::size_t gap_tolerance,
	std::size_t byte_budget,
	std::size_t page_size = test_page
)
{
	const mrc_prefetch_policy policy = {
		gap_tolerance, byte_budget, page_size
	};

	return policy;
}

// Past the last plane of the fixture, so nothing is clamped unless a case
// asks for it.
std::size_t whole_file(const mrc_geometry &geometry)
{
	return geometry.get_data_offset() + geometry.get_data_size();
}

} // anonymous namespace

TEST_CASE( "what one region of a batch spans is its bounding stretch",
	"[mrc_region_prefetch_plan]" )
{
	const auto geometry = make_stack_geometry();

	SECTION( "a whole plane spans the plane" )
	{
		const image_transfer_plan regions(make_span(plane), 3, 3);

		REQUIRE( compute_region_span(regions, geometry) == plane_bytes );
	}

	SECTION( "a crop spans its bounding box rather than its elements" )
	{
		const std::vector<std::size_t> crop = {2, 2};
		const image_transfer_plan regions(make_span(crop), 3, 3);

		// Two rows of two, four elements apart: the first to the last is
		// six elements, not the four it holds.
		REQUIRE( compute_region_span(regions, geometry) ==
			6 * sizeof(float) );
	}

	SECTION( "a region of no elements spans nothing" )
	{
		const std::vector<std::size_t> empty = {0, 4};
		const image_transfer_plan regions(make_span(empty), 3, 3);

		REQUIRE( compute_region_span(regions, geometry) == 0 );
	}
}

TEST_CASE( "a batch is advised as the stretches it reaches",
	"[mrc_region_prefetch_plan]" )
{
	const auto geometry = make_stack_geometry();
	const auto values = geometry.get_data_offset();
	const auto mapped = whole_file(geometry);

	SECTION( "a batch of no region is advised nothing" )
	{
		const image_transfer_plan regions(make_span(plane), 3, 3);
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_step_count() == 0 );
		CHECK( advice.get_ranges().empty() );
	}

	SECTION( "one region is the stretch it spans" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 2);

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({2})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_offset() ==
			values + 2 * plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
	}

	SECTION( "consecutive regions merge into one stretch" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 1);
		add_plane(regions, 2);
		add_plane(regions, 3);

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({1, 2, 3})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_offset() == values + plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == 3 * plane_bytes );
	}

	SECTION( "regions with a gap between them leave the gap out" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 1);
		add_plane(regions, 4);

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({1, 4})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 2 );
		CHECK( advice.get_ranges()[0].get_offset() == values + plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
		CHECK( advice.get_ranges()[1].get_offset() ==
			values + 4 * plane_bytes );
		CHECK( advice.get_ranges()[1].get_size() == plane_bytes );
	}

	SECTION( "a gap within the tolerance is bridged" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 0);
		add_plane(regions, 2);

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({0, 2})), mapped,
			make_policy(plane_bytes, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_offset() == values );
		CHECK( advice.get_ranges()[0].get_size() == 3 * plane_bytes );
	}

	SECTION( "a region stated twice is advised once" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 3);
		add_plane(regions, 3);

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({3, 3})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
	}

	SECTION( "a region of no elements is advised nothing" )
	{
		const std::vector<std::size_t> empty = {0, 4};
		image_transfer_plan regions(make_span(empty), 3, 3);
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));

		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({0})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_step_count() == 0 );
		CHECK( advice.get_ranges().empty() );
	}
}

TEST_CASE( "every stretch starts on a page and stays within the mapping",
	"[mrc_region_prefetch_plan]" )
{
	const auto geometry = make_stack_geometry();
	const auto values = geometry.get_data_offset();

	image_transfer_plan regions(make_span(plane), 3, 3);
	add_plane(regions, 1);

	SECTION( "a stretch starting inside a page grows back to its boundary" )
	{
		// A plane is 48 bytes, so the second one starts 16 bytes into a
		// page of 32.
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({1})),
			whole_file(geometry), make_policy(0, default_prefetch_budget, 32)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_offset() ==
			values + plane_bytes - 16 );
		CHECK( advice.get_ranges()[0].get_offset() % 32 == 0 );
	}

	SECTION( "a stretch is cut short where the mapping ends" )
	{
		const auto mapped = values + plane_bytes + 16;
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({1})), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_offset() +
			advice.get_ranges()[0].get_size() == mapped );
	}

	SECTION( "a region starting past the mapping is left out" )
	{
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(plane_offsets({1})), values,
			make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_ranges().empty() );
		CHECK( advice.get_step_count() == 0 );
	}
}

TEST_CASE( "the stretches of a batch are grouped into steps",
	"[mrc_region_prefetch_plan]" )
{
	const auto geometry = make_stack_geometry();
	const auto mapped = whole_file(geometry);

	image_transfer_plan regions(make_span(plane), 3, 3);
	add_plane(regions, 0);
	add_plane(regions, 2);
	add_plane(regions, 4);

	const auto offsets = plane_offsets({0, 2, 4});

	SECTION( "a batch within the budget is one step" )
	{
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(offsets), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_step_count() == 1 );
		CHECK( advice.get_step_first_region(0) == 0 );
		CHECK( advice.get_step_region_count(0) == 3 );
		CHECK( advice.get_step_ranges(0).size() == 3 );
	}

	SECTION( "a budget of one stretch is one step each" )
	{
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(offsets), mapped,
			make_policy(0, plane_bytes)
		);

		REQUIRE( advice.get_step_count() == 3 );
		for (std::size_t step = 0; step < 3; ++step)
		{
			CHECK( advice.get_step_first_region(step) == step );
			CHECK( advice.get_step_region_count(step) == 1 );
			CHECK( advice.get_step_ranges(step).size() == 1 );
		}
	}

	SECTION( "a step holds a region wider than the budget" )
	{
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(offsets), mapped,
			make_policy(0, 1)
		);

		REQUIRE( advice.get_step_count() == 3 );
		CHECK( advice.get_step_region_count(0) == 1 );
		CHECK( advice.get_step_ranges(0).size() == 1 );
	}

	SECTION( "the steps cover every region once, in order" )
	{
		const mrc_region_prefetch_plan advice(
			regions, geometry, make_span(offsets), mapped,
			make_policy(0, 2 * plane_bytes)
		);

		std::size_t covered = 0;
		for (std::size_t step = 0; step < advice.get_step_count(); ++step)
		{
			CHECK( advice.get_step_first_region(step) == covered );
			covered += advice.get_step_region_count(step);
		}

		CHECK( covered == 3 );
	}
}

TEST_CASE( "the default policy scales its tolerance with the region",
	"[mrc_region_prefetch_plan]" )
{
	SECTION( "a region wider than the cap is capped" )
	{
		const auto policy =
			make_prefetch_policy(4 * default_prefetch_gap_cap);

		CHECK( policy.gap_tolerance == default_prefetch_gap_cap );
	}

	SECTION( "a region narrower than the cap sets the tolerance" )
	{
		// Floored at a page, so the region has to be wider than one to be
		// what decides it.
		const std::size_t span = default_prefetch_gap_cap / 2;
		const auto policy = make_prefetch_policy(span);

		CHECK( policy.gap_tolerance <= default_prefetch_gap_cap );
		CHECK( policy.gap_tolerance >= span );
	}

	SECTION( "it takes the page size of the machine" )
	{
		const auto policy = make_prefetch_policy(plane_bytes);

		CHECK( policy.page_size == get_page_size() );
		CHECK( policy.byte_budget == default_prefetch_budget );
	}
}
