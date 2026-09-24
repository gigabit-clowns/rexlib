// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/strided_transfer/image_region_offsets.hpp>

#include <rexlib/em/image/image_transfer_plan.hpp>

#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<std::ptrdiff_t> collect(span<const std::ptrdiff_t> values)
{
	return std::vector<std::ptrdiff_t>(values.begin(), values.end());
}

} // anonymous namespace

TEST_CASE( "where a region starts is the offsets times the strides",
	"[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {4, 4};
	const std::vector<std::ptrdiff_t> strides = {4, 1};
	const std::vector<std::size_t> region = {2, 2};

	image_transfer_plan regions(make_span(region), 2, 2);
	regions.add(make_span(std::vector<std::size_t>{1, 2}),
		make_span(std::vector<std::size_t>{2, 1}));

	const image_region_offsets offsets(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "each side is resolved with its own strides" )
	{
		REQUIRE( collect(offsets.get_file()) ==
			std::vector<std::ptrdiff_t>{1 * 4 + 2 * 1} );
		REQUIRE( collect(offsets.get_array()) ==
			std::vector<std::ptrdiff_t>{2 * 4 + 1 * 1} );
	}

	SECTION( "one region is counted" )
	{
		REQUIRE( offsets.get_region_count() == 1 );
	}
}

TEST_CASE( "the array carries the offset of its own first element",
	"[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {4, 4};
	const std::vector<std::ptrdiff_t> strides = {4, 1};
	const std::vector<std::size_t> region = {2, 2};

	image_transfer_plan regions(make_span(region), 2, 2);
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{1, 1}));

	const image_region_offsets offsets(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		7
	);

	SECTION( "it is added to every array offset" )
	{
		REQUIRE( collect(offsets.get_array()) ==
			std::vector<std::ptrdiff_t>{7 + 4 + 1} );
	}

	SECTION( "the file has no such offset of its own" )
	{
		REQUIRE( collect(offsets.get_file()) ==
			std::vector<std::ptrdiff_t>{0} );
	}
}

TEST_CASE( "a side deeper than the region resolves its leading axes too",
	"[image_region_offsets]" )
{
	// The extents of a batch cover the trailing axes; the leading ones span
	// a single position and are what a stack is indexed along.
	const std::vector<std::size_t> file_extents = {5, 3, 4};
	const std::vector<std::ptrdiff_t> file_strides = {12, 4, 1};
	const std::vector<std::size_t> array_extents = {3, 4};
	const std::vector<std::ptrdiff_t> array_strides = {4, 1};
	const std::vector<std::size_t> region = {3, 4};

	image_transfer_plan regions(make_span(region), 3, 2);
	regions.add(make_span(std::vector<std::size_t>{2, 0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	const image_region_offsets offsets(
		regions,
		make_span(file_extents), make_span(file_strides),
		make_span(array_extents), make_span(array_strides),
		0
	);

	REQUIRE( collect(offsets.get_file()) ==
		std::vector<std::ptrdiff_t>{2 * 12} );
	REQUIRE( collect(offsets.get_array()) ==
		std::vector<std::ptrdiff_t>{0} );
}

TEST_CASE( "every region of a batch is resolved", "[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {3, 2, 2};
	const std::vector<std::ptrdiff_t> strides = {4, 2, 1};
	const std::vector<std::size_t> region = {2, 2};

	image_transfer_plan regions(make_span(region), 3, 3);
	for (std::size_t i = 0; i < 3; ++i)
	{
		regions.add(
			make_span(std::vector<std::size_t>{i, 0, 0}),
			make_span(std::vector<std::size_t>{2 - i, 0, 0})
		);
	}

	const image_region_offsets offsets(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	REQUIRE( offsets.get_region_count() == 3 );
	REQUIRE( collect(offsets.get_file()) ==
		std::vector<std::ptrdiff_t>{0, 4, 8} );
	REQUIRE( collect(offsets.get_array()) ==
		std::vector<std::ptrdiff_t>{8, 4, 0} );
}

TEST_CASE( "a batch is held in ascending file order",
	"[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {3, 2, 2};
	const std::vector<std::ptrdiff_t> strides = {4, 2, 1};
	const std::vector<std::size_t> region = {2, 2};

	// Stated 2, 0, 1 along the file, each paired with a different plane of
	// the array, so that the order and the pairing are told apart.
	const std::size_t positions[3] = {2, 0, 1};

	image_transfer_plan regions(make_span(region), 3, 3);
	for (std::size_t i = 0; i < 3; ++i)
	{
		regions.add(
			make_span(std::vector<std::size_t>{positions[i], 0, 0}),
			make_span(std::vector<std::size_t>{i, 0, 0})
		);
	}

	const image_region_offsets offsets(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	SECTION( "the file offsets come out ascending" )
	{
		REQUIRE( collect(offsets.get_file()) ==
			std::vector<std::ptrdiff_t>{0, 4, 8} );
	}

	SECTION( "each array offset follows the file offset it was stated with" )
	{
		REQUIRE( collect(offsets.get_array()) ==
			std::vector<std::ptrdiff_t>{4, 8, 0} );
	}
}

TEST_CASE( "an empty batch resolves to nothing", "[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::ptrdiff_t> strides = {2, 1};

	const image_transfer_plan regions(make_span(extents), 2, 2);
	const image_region_offsets offsets(
		regions,
		make_span(extents), make_span(strides),
		make_span(extents), make_span(strides),
		0
	);

	REQUIRE( offsets.get_region_count() == 0 );
	REQUIRE( offsets.get_array().empty() );
	REQUIRE( offsets.get_file().empty() );
}

TEST_CASE( "a region that does not fit is refused", "[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::ptrdiff_t> strides = {2, 1};
	const std::vector<std::size_t> region = {2, 2};

	const auto build = [&] (
		const std::vector<std::size_t> &file_offset,
		const std::vector<std::size_t> &array_offset
	)
	{
		image_transfer_plan regions(make_span(region), 2, 2);
		regions.add(make_span(file_offset), make_span(array_offset));

		return image_region_offsets(
			regions,
			make_span(extents), make_span(strides),
			make_span(extents), make_span(strides),
			0
		);
	};

	SECTION( "one past the end of the file is refused" )
	{
		REQUIRE_THROWS_AS( build({1, 0}, {0, 0}), std::out_of_range );
	}

	SECTION( "one past the end of the array is refused" )
	{
		REQUIRE_THROWS_AS( build({0, 0}, {0, 1}), std::out_of_range );
	}

	SECTION( "one that fits exactly is not" )
	{
		REQUIRE_NOTHROW( build({0, 0}, {0, 0}) );
	}
}

TEST_CASE( "a batch whose ranks disagree with its sides is refused",
	"[image_region_offsets]" )
{
	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<std::ptrdiff_t> strides = {2, 1};
	const std::vector<std::size_t> deeper = {1, 2, 2};
	const std::vector<std::ptrdiff_t> deeper_strides = {4, 2, 1};

	image_transfer_plan regions(make_span(extents), 2, 2);
	regions.add(make_span(std::vector<std::size_t>{0, 0}),
		make_span(std::vector<std::size_t>{0, 0}));

	SECTION( "file extents of the wrong rank are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_offsets(
				regions,
				make_span(deeper), make_span(deeper_strides),
				make_span(extents), make_span(strides),
				0
			),
			std::invalid_argument
		);
	}

	SECTION( "array extents of the wrong rank are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_offsets(
				regions,
				make_span(extents), make_span(strides),
				make_span(deeper), make_span(deeper_strides),
				0
			),
			std::invalid_argument
		);
	}

	SECTION( "file strides that do not match their extents are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_offsets(
				regions,
				make_span(extents), make_span(deeper_strides),
				make_span(extents), make_span(strides),
				0
			),
			std::invalid_argument
		);
	}

	SECTION( "array strides that do not match their extents are refused" )
	{
		REQUIRE_THROWS_AS(
			image_region_offsets(
				regions,
				make_span(extents), make_span(strides),
				make_span(extents), make_span(deeper_strides),
				0
			),
			std::invalid_argument
		);
	}
}
