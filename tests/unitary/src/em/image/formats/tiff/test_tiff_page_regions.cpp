// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/tiff/tiff_page_regions.hpp>

#include <rexlib/core/span.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::tiff;

namespace
{

using offset = std::vector<std::size_t>;

void add(
	image_transfer_plan &regions,
	const offset &file_offset,
	const offset &array_offset
)
{
	regions.add(make_span(file_offset), make_span(array_offset));
}

offset file_offset_of(const image_transfer_plan &regions, std::size_t index)
{
	const auto values = regions.get_file_offset(index);
	return offset(values.begin(), values.end());
}

offset array_offset_of(const image_transfer_plan &regions, std::size_t index)
{
	const auto values = regions.get_array_offset(index);
	return offset(values.begin(), values.end());
}

offset extents_of(const image_transfer_plan &regions)
{
	const auto values = regions.get_shape().get_extents();
	return offset(values.begin(), values.end());
}

} // anonymous namespace

TEST_CASE( "regions of a single page stay on it",
	"[tiff_page_regions]" )
{
	image_transfer_plan regions(image_transfer_shape({4, 5}, 2, 3));
	add(regions, {10, 20}, {0, 0, 0});
	add(regions, {2, 30}, {1, 0, 0});

	const tiff_page_regions pages(regions);

	SECTION( "the one page is reached" )
	{
		REQUIRE( pages.get_page_count() == 1 );
		REQUIRE( pages.get_page(0) == 0 );
	}

	SECTION( "its regions are the ones of the plan" )
	{
		const auto &page = pages.get_regions(0);

		REQUIRE( page.get_region_count() == 2 );
		REQUIRE( extents_of(page) == offset{4, 5} );
		REQUIRE( page.get_shape().get_file_rank() == 2 );
		REQUIRE( page.get_shape().get_array_rank() == 3 );
		REQUIRE( file_offset_of(page, 0) == offset{10, 20} );
		REQUIRE( array_offset_of(page, 0) == offset{0, 0, 0} );
		REQUIRE( file_offset_of(page, 1) == offset{2, 30} );
		REQUIRE( array_offset_of(page, 1) == offset{1, 0, 0} );
	}

	SECTION( "the rows reached run from the first region to the last" )
	{
		REQUIRE( pages.get_first_row(0) == 2 );
		REQUIRE( pages.get_row_count(0) == 12 );
	}
}

TEST_CASE( "regions of a stack are gathered by the page they lie on",
	"[tiff_page_regions]" )
{
	image_transfer_plan regions(image_transfer_shape({4, 5}, 3, 3));
	add(regions, {7, 10, 20}, {0, 0, 0});
	add(regions, {2, 0, 0}, {1, 0, 0});
	add(regions, {7, 1, 3}, {2, 0, 0});

	const tiff_page_regions pages(regions);

	SECTION( "the pages reached ascend, each one once" )
	{
		REQUIRE( pages.get_page_count() == 2 );
		REQUIRE( pages.get_page(0) == 2 );
		REQUIRE( pages.get_page(1) == 7 );
	}

	SECTION( "a region loses the page axis and keeps where it lands" )
	{
		const auto &first = pages.get_regions(0);

		REQUIRE( first.get_region_count() == 1 );
		REQUIRE( first.get_shape().get_file_rank() == 2 );
		REQUIRE( first.get_shape().get_array_rank() == 3 );
		REQUIRE( extents_of(first) == offset{4, 5} );
		REQUIRE( file_offset_of(first, 0) == offset{0, 0} );
		REQUIRE( array_offset_of(first, 0) == offset{1, 0, 0} );
	}

	SECTION( "the regions of one page keep the order of the plan" )
	{
		const auto &second = pages.get_regions(1);

		REQUIRE( second.get_region_count() == 2 );
		REQUIRE( file_offset_of(second, 0) == offset{10, 20} );
		REQUIRE( array_offset_of(second, 0) == offset{0, 0, 0} );
		REQUIRE( file_offset_of(second, 1) == offset{1, 3} );
		REQUIRE( array_offset_of(second, 1) == offset{2, 0, 0} );
	}

	SECTION( "each page states the rows its own regions reach" )
	{
		REQUIRE( pages.get_first_row(0) == 0 );
		REQUIRE( pages.get_row_count(0) == 4 );
		REQUIRE( pages.get_first_row(1) == 1 );
		REQUIRE( pages.get_row_count(1) == 13 );
	}
}

TEST_CASE( "a region spanning pages becomes one region on each",
	"[tiff_page_regions]" )
{
	// Three pages from the second one on, landing from the fifth position
	// of the axis of the array that runs along them, which is its second.
	image_transfer_plan regions(image_transfer_shape({3, 4, 5}, 3, 4));
	add(regions, {1, 6, 7}, {9, 4, 0, 2});

	const tiff_page_regions pages(regions);

	SECTION( "every page it spans is reached" )
	{
		REQUIRE( pages.get_page_count() == 3 );
		REQUIRE( pages.get_page(0) == 1 );
		REQUIRE( pages.get_page(1) == 2 );
		REQUIRE( pages.get_page(2) == 3 );
	}

	SECTION( "the region of a page has the extents of a page" )
	{
		REQUIRE( extents_of(pages.get_regions(0)) == offset{4, 5} );
		REQUIRE( pages.get_regions(0).get_shape().get_file_rank() == 2 );
		REQUIRE( pages.get_regions(0).get_shape().get_array_rank() == 4 );
	}

	SECTION( "each page lands one position further along the array" )
	{
		for (std::size_t position = 0; position < 3; ++position)
		{
			const auto &page = pages.get_regions(position);

			REQUIRE( page.get_region_count() == 1 );
			REQUIRE( file_offset_of(page, 0) == offset{6, 7} );
			REQUIRE( array_offset_of(page, 0) ==
				offset{9, 4 + position, 0, 2} );
			REQUIRE( pages.get_first_row(position) == 6 );
			REQUIRE( pages.get_row_count(position) == 4 );
		}
	}

	SECTION( "regions that overlap in pages share the ones they both span" )
	{
		add(regions, {3, 0, 0}, {0, 0, 0, 0});

		const tiff_page_regions overlapping(regions);

		REQUIRE( overlapping.get_page_count() == 5 );
		REQUIRE( overlapping.get_page(4) == 5 );
		REQUIRE( overlapping.get_regions(1).get_region_count() == 1 );
		REQUIRE( overlapping.get_regions(2).get_region_count() == 2 );
		REQUIRE( array_offset_of(overlapping.get_regions(2), 1) ==
			offset{0, 0, 0, 0} );
		REQUIRE( overlapping.get_first_row(2) == 0 );
		REQUIRE( overlapping.get_row_count(2) == 10 );
	}
}

TEST_CASE( "regions of lower rank than a page reach a single row",
	"[tiff_page_regions]" )
{
	SECTION( "a run of one row" )
	{
		image_transfer_plan regions(image_transfer_shape({5}, 3, 1));
		add(regions, {4, 8, 2}, {0});

		const tiff_page_regions pages(regions);

		REQUIRE( pages.get_page_count() == 1 );
		REQUIRE( pages.get_page(0) == 4 );
		REQUIRE( extents_of(pages.get_regions(0)) == offset{5} );
		REQUIRE( file_offset_of(pages.get_regions(0), 0) == offset{8, 2} );
		REQUIRE( array_offset_of(pages.get_regions(0), 0) == offset{0} );
		REQUIRE( pages.get_first_row(0) == 8 );
		REQUIRE( pages.get_row_count(0) == 1 );
	}

	SECTION( "a single sample" )
	{
		image_transfer_plan regions(image_transfer_shape({}, 2, 0));
		add(regions, {3, 9}, {});

		const tiff_page_regions pages(regions);

		REQUIRE( pages.get_page_count() == 1 );
		REQUIRE( pages.get_first_row(0) == 3 );
		REQUIRE( pages.get_row_count(0) == 1 );
	}
}

TEST_CASE( "a plan of no region reaches no page",
	"[tiff_page_regions]" )
{
	const image_transfer_plan regions(image_transfer_shape({4, 5}, 3, 2));

	const tiff_page_regions pages(regions);

	REQUIRE( pages.get_page_count() == 0 );
}

TEST_CASE( "regions against anything but a page or a stack are refused",
	"[tiff_page_regions]" )
{
	SECTION( "a file of one axis" )
	{
		const image_transfer_plan regions(image_transfer_shape({5}, 1, 1));

		REQUIRE_THROWS_AS( tiff_page_regions(regions), std::invalid_argument );
	}

	SECTION( "a file of four axes" )
	{
		const image_transfer_plan regions(
			image_transfer_shape({4, 5}, 4, 2));

		REQUIRE_THROWS_AS( tiff_page_regions(regions), std::invalid_argument );
	}
}
