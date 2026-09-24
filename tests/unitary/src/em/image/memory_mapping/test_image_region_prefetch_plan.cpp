// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/memory_mapping/image_region_prefetch_plan.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A stack of five 3x4 planes of float32 whose values begin 1024 bytes into
// the file, so one plane is 48 bytes.
const std::vector<std::ptrdiff_t> stack_strides = {12, 4, 1};
REXLIB_CONST_CONSTEXPR std::size_t values_offset = 1024;
REXLIB_CONST_CONSTEXPR std::size_t plane_bytes = 3 * 4 * sizeof(float);

// Past the last plane of the fixture, so nothing is clamped unless a case
// asks for it.
REXLIB_CONST_CONSTEXPR std::size_t whole_file = values_offset + 5 * plane_bytes;

// Small enough that the planes of the fixture do not all land in one page,
// which the page of a machine would make them do.
REXLIB_CONST_CONSTEXPR std::size_t test_page = 16;

// Where the file of the fixture is mapped is a multiple of every page the
// cases use, as a mapping is at a multiple of the page of the machine.
REXLIB_CONST_CONSTEXPR std::size_t mapping_alignment = 64;

const std::vector<std::size_t> plane = {3, 4};

void add_plane(image_transfer_plan &regions, std::size_t section)
{
	const std::size_t file_offset[3] = {section, 0, 0};
	const std::size_t array_offset[3] = {0, 0, 0};
	regions.add(make_span(file_offset, 3), make_span(array_offset, 3));
}

// Where the planes named start in the file, in elements and ascending, which
// is how image_region_offsets holds them.
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

image_prefetch_policy make_policy(
	std::size_t gap_tolerance,
	std::size_t byte_budget,
	std::size_t page_size = test_page
)
{
	return image_prefetch_policy(gap_tolerance, byte_budget, page_size);
}

std::size_t span_of(const image_transfer_plan &regions)
{
	return compute_region_span(
		regions,
		make_span(stack_strides),
		numerical_type::float32
	);
}

image_region_prefetch_plan advise(
	const image_transfer_plan &regions,
	const std::vector<std::ptrdiff_t> &file_offsets,
	span<rexlib::byte> mapping,
	const image_prefetch_policy &policy
)
{
	return image_region_prefetch_plan(
		regions,
		make_span(stack_strides),
		numerical_type::float32,
		make_span(file_offsets),
		mapping,
		values_offset,
		policy
	);
}

std::unique_ptr<host_buffer> map_file()
{
	return std::make_unique<host_buffer>(whole_file, mapping_alignment);
}

// The first bytes of a mapping, which is how a case makes one end early.
span<rexlib::byte> first_bytes(host_buffer &file, std::size_t size)
{
	return make_span(static_cast<rexlib::byte*>(file.get_host_ptr()), size);
}

std::size_t offset_in(const memory_range &range, const host_buffer &file)
{
	return reinterpret_cast<std::uintptr_t>(range.get_address()) -
		reinterpret_cast<std::uintptr_t>(file.get_host_ptr());
}

} // anonymous namespace

TEST_CASE( "what one region of a batch spans is its bounding stretch",
	"[image_region_prefetch_plan]" )
{

	SECTION( "a whole plane spans the plane" )
	{
		const image_transfer_plan regions(make_span(plane), 3, 3);

		REQUIRE( span_of(regions) == plane_bytes );
	}

	SECTION( "a crop spans its bounding box rather than its elements" )
	{
		const std::vector<std::size_t> crop = {2, 2};
		const image_transfer_plan regions(make_span(crop), 3, 3);

		// Two rows of two, four elements apart: the first to the last is
		// six elements, not the four it holds.
		REQUIRE( span_of(regions) ==
			6 * sizeof(float) );
	}

	SECTION( "a region of no elements spans nothing" )
	{
		const std::vector<std::size_t> empty = {0, 4};
		const image_transfer_plan regions(make_span(empty), 3, 3);

		REQUIRE( span_of(regions) == 0 );
	}
}

TEST_CASE( "a batch is advised as the stretches it reaches",
	"[image_region_prefetch_plan]" )
{
	const auto values = values_offset;
	const auto file = map_file();
	const auto mapped = first_bytes(*file, whole_file);

	SECTION( "a batch of no region is advised nothing" )
	{
		const image_transfer_plan regions(make_span(plane), 3, 3);
		const auto advice = advise(
			regions, plane_offsets({}), mapped,
			make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_step_count() == 0 );
		CHECK( advice.get_ranges().empty() );
	}

	SECTION( "one region is the stretch it spans" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 2);

		const auto advice = advise(
			regions, plane_offsets({2}), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( offset_in(advice.get_ranges()[0], *file) ==
			values + 2 * plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
	}

	SECTION( "consecutive regions merge into one stretch" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 1);
		add_plane(regions, 2);
		add_plane(regions, 3);

		const auto advice = advise(
			regions, plane_offsets({1, 2, 3}), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( offset_in(advice.get_ranges()[0], *file) ==
			values + plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == 3 * plane_bytes );
	}

	SECTION( "regions with a gap between them leave the gap out" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 1);
		add_plane(regions, 4);

		const auto advice = advise(
			regions, plane_offsets({1, 4}), mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 2 );
		CHECK( offset_in(advice.get_ranges()[0], *file) ==
			values + plane_bytes );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
		CHECK( offset_in(advice.get_ranges()[1], *file) ==
			values + 4 * plane_bytes );
		CHECK( advice.get_ranges()[1].get_size() == plane_bytes );
	}

	SECTION( "a gap within the tolerance is bridged" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 0);
		add_plane(regions, 2);

		const auto advice = advise(
			regions, plane_offsets({0, 2}), mapped,
			make_policy(plane_bytes, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( offset_in(advice.get_ranges()[0], *file) == values );
		CHECK( advice.get_ranges()[0].get_size() == 3 * plane_bytes );
	}

	SECTION( "a region stated twice is advised once" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 3);
		add_plane(regions, 3);

		const auto advice = advise(
			regions, plane_offsets({3, 3}), mapped,
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

		const auto advice = advise(
			regions, plane_offsets({0}), mapped,
			make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_ranges().empty() );

		// Nothing to ask for, but the region is still one a read moves.
		REQUIRE( advice.get_step_count() == 1 );
		CHECK( advice.get_step_ranges(0).empty() );
		CHECK( advice.get_step_region_count(0) == 1 );
	}
}

TEST_CASE( "every stretch starts on a page and stays within the mapping",
	"[image_region_prefetch_plan]" )
{
	const auto values = values_offset;
	const auto file = map_file();

	image_transfer_plan regions(make_span(plane), 3, 3);
	add_plane(regions, 1);

	SECTION( "a stretch starting inside a page grows back to its boundary" )
	{
		// A plane is 48 bytes, so the second one starts 16 bytes into a
		// page of 32.
		const auto advice = advise(
			regions, plane_offsets({1}),
			first_bytes(*file, whole_file),
			make_policy(0, default_prefetch_budget, 32)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		const auto range = advice.get_ranges()[0];
		CHECK( reinterpret_cast<std::uintptr_t>(range.get_address()) % 32 ==
			0 );
		CHECK( offset_in(range, *file) == values + plane_bytes - 16 );
	}

	SECTION( "a stretch is cut short where the mapping ends" )
	{
		const auto end = values + plane_bytes + 16;
		const auto advice = advise(
			regions, plane_offsets({1}),
			first_bytes(*file, end), make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		const auto range = advice.get_ranges()[0];
		CHECK( offset_in(range, *file) + range.get_size() == end );
	}

	SECTION( "a region starting past the mapping is asked for nothing" )
	{
		const auto advice = advise(
			regions, plane_offsets({1}),
			first_bytes(*file, values), make_policy(0, default_prefetch_budget)
		);

		CHECK( advice.get_ranges().empty() );

		// It is left out of the stretches but not out of the steps: what is
		// advised is a hint, what is moved is not.
		REQUIRE( advice.get_step_count() == 1 );
		CHECK( advice.get_step_region_count(0) == 1 );
	}

	SECTION( "a region past the mapping still belongs to the last step" )
	{
		image_transfer_plan both(make_span(plane), 3, 3);
		add_plane(both, 0);
		add_plane(both, 1);

		// Only the first plane is mapped, so the second is asked for
		// nothing yet still has to be moved.
		const auto advice = advise(
			both, plane_offsets({0, 1}),
			first_bytes(*file, values + plane_bytes),
			make_policy(0, default_prefetch_budget)
		);

		std::size_t covered = 0;
		for (std::size_t step = 0; step < advice.get_step_count(); ++step)
		{
			covered += advice.get_step_region_count(step);
		}

		CHECK( covered == 2 );
	}
}

TEST_CASE( "the stretches of a batch are grouped into steps",
	"[image_region_prefetch_plan]" )
{
	const auto file = map_file();
	const auto mapped = first_bytes(*file, whole_file);

	image_transfer_plan regions(make_span(plane), 3, 3);
	add_plane(regions, 0);
	add_plane(regions, 2);
	add_plane(regions, 4);

	const auto offsets = plane_offsets({0, 2, 4});

	SECTION( "a batch within the budget is one step" )
	{
		const auto advice = advise(
			regions, offsets, mapped,
			make_policy(0, default_prefetch_budget)
		);

		REQUIRE( advice.get_step_count() == 1 );
		CHECK( advice.get_step_first_region(0) == 0 );
		CHECK( advice.get_step_region_count(0) == 3 );
		CHECK( advice.get_step_ranges(0).size() == 3 );
	}

	SECTION( "a budget of one stretch is one step each" )
	{
		const auto advice = advise(
			regions, offsets, mapped,
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
		const auto advice = advise(
			regions, offsets, mapped,
			make_policy(0, 1)
		);

		REQUIRE( advice.get_step_count() == 3 );
		CHECK( advice.get_step_region_count(0) == 1 );
		CHECK( advice.get_step_ranges(0).size() == 1 );
	}

	SECTION( "the steps cover every region once, in order" )
	{
		const auto advice = advise(
			regions, offsets, mapped,
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

TEST_CASE( "merging never grows a stretch past the budget of a step",
	"[image_region_prefetch_plan]" )
{
	const auto file = map_file();
	const auto mapped = first_bytes(*file, whole_file);

	SECTION( "a long run of consecutive regions is split into steps" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		for (std::size_t section = 0; section < 5; ++section)
		{
			add_plane(regions, section);
		}

		const auto advice = advise(
			regions, plane_offsets({0, 1, 2, 3, 4}),
			mapped, make_policy(0, 2 * plane_bytes)
		);

		REQUIRE( advice.get_ranges().size() == 3 );
		CHECK( advice.get_ranges()[0].get_size() == 2 * plane_bytes );
		CHECK( advice.get_ranges()[1].get_size() == 2 * plane_bytes );
		CHECK( advice.get_ranges()[2].get_size() == plane_bytes );

		REQUIRE( advice.get_step_count() == 3 );
		CHECK( advice.get_step_first_region(0) == 0 );
		CHECK( advice.get_step_region_count(0) == 2 );
		CHECK( advice.get_step_first_region(1) == 2 );
		CHECK( advice.get_step_region_count(1) == 2 );
		CHECK( advice.get_step_first_region(2) == 4 );
		CHECK( advice.get_step_region_count(2) == 1 );
	}

	SECTION( "a region wider than the budget takes in no neighbour" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 1);
		add_plane(regions, 2);

		const auto advice = advise(
			regions, plane_offsets({1, 2}), mapped,
			make_policy(0, 1)
		);

		CHECK( advice.get_ranges().size() == 2 );
		CHECK( advice.get_step_count() == 2 );
	}

	SECTION( "a region stated twice is advised once whatever the budget" )
	{
		image_transfer_plan regions(make_span(plane), 3, 3);
		add_plane(regions, 3);
		add_plane(regions, 3);

		const auto advice = advise(
			regions, plane_offsets({3, 3}), mapped,
			make_policy(0, 1)
		);

		REQUIRE( advice.get_ranges().size() == 1 );
		CHECK( advice.get_ranges()[0].get_size() == plane_bytes );
		REQUIRE( advice.get_step_count() == 1 );
		CHECK( advice.get_step_region_count(0) == 2 );
	}
}
