// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_geometry.hpp>

#include <rexlib/em/image/exceptions/image_format_error.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

mrc_header make_header_of(
	std::int32_t columns,
	std::int32_t rows,
	std::int32_t sections,
	std::int32_t section_sampling,
	std::int32_t space_group,
	mrc_mode mode = mrc_mode::float32
)
{
	mrc_header header;
	header.set_column_count(columns);
	header.set_row_count(rows);
	header.set_section_count(sections);
	header.set_section_sampling(section_sampling);
	header.set_space_group(space_group);
	header.set_mode(mode);
	header.set_column_axis(1);
	header.set_row_axis(2);
	header.set_section_axis(3);
	return header;
}

mrc_header with_axes(
	mrc_header header,
	std::int32_t column_axis,
	std::int32_t row_axis,
	std::int32_t section_axis
)
{
	header.set_column_axis(column_axis);
	header.set_row_axis(row_axis);
	header.set_section_axis(section_axis);
	return header;
}

std::vector<std::size_t> extents_of(const mrc_geometry &geometry)
{
	const auto extents = geometry.get_descriptor().get_extents();
	return std::vector<std::size_t>(extents.begin(), extents.end());
}

std::vector<std::ptrdiff_t> strides_of(const mrc_geometry &geometry)
{
	const auto strides = geometry.get_strides();
	return std::vector<std::ptrdiff_t>(strides.begin(), strides.end());
}

} // anonymous namespace

TEST_CASE( "the shape of an MRC file follows from its space group",
	"[mrc_geometry]" )
{
	SECTION( "one section and no space group is a single image" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 1, 1, 0));

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "several sections and no space group is a stack of images" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{5, 3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "a space group of one is a volume" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 5, 5, 1));

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{5, 3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a crystallographic space group is a volume too" )
	{
		const mrc_geometry geometry(make_header_of(73, 43, 25, 72, 4));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{25, 43, 73} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a space group above four hundred is a stack of volumes" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 12, 4, 401));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{3, 4, 3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "the last space group of the range is still a stack" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 12, 4, 630));

		REQUIRE( extents_of(geometry).size() == 4 );
	}

	SECTION( "one past the range is a volume" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 12, 4, 631));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{12, 3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}
}

TEST_CASE( "a crystallographic file does not read its sampling as a depth",
	"[mrc_geometry]" )
{
	// The sampling of EMD-3001, which is unrelated to its section count.
	mrc_header header = make_header_of(73, 43, 25, 72, 4);
	header.set_column_sampling(40);
	header.set_row_sampling(12);

	const mrc_geometry geometry(header);

	REQUIRE( extents_of(geometry) == std::vector<std::size_t>{25, 43, 73} );
	REQUIRE( geometry.get_element_count() == 25 * 43 * 73 );
}

TEST_CASE( "the values of an MRC file are laid out contiguously",
	"[mrc_geometry]" )
{
	SECTION( "a single image counts by rows" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 1, 1, 0));

		REQUIRE( strides_of(geometry) == std::vector<std::ptrdiff_t>{4, 1} );
	}

	SECTION( "a stack counts by sections and rows" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{12, 4, 1} );
	}

	SECTION( "a stack of volumes counts by volumes as well" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 12, 4, 401));

		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{48, 12, 4, 1} );
	}
}

TEST_CASE(
	"a stack of volumes is one however few its volumes or sections",
	"[mrc_geometry]"
)
{
	// MRC2014 states a stack of volumes by its space group alone.
	SECTION( "volumes one section deep" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 6, 1, 401));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{6, 1, 3, 4} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{12, 12, 4, 1} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a single volume" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 5, 5, 401));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{1, 5, 3, 4} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{60, 12, 4, 1} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a single volume of a single section" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 1, 1, 401));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{1, 1, 3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}
}

TEST_CASE(
	"a single section in the image space group is read as the caller says",
	"[mrc_geometry]"
)
{
	// MRC2014 states a single image and a stack of one image alike.
	const auto header = make_header_of(4, 3, 1, 1, 0);

	SECTION( "a single image by default" )
	{
		const mrc_geometry geometry(header);

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{3, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "a stack of one image when told so" )
	{
		const mrc_geometry geometry(header, mrc_single_section::image_stack);

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{1, 3, 4} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{12, 4, 1} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "more than one section is a stack either way" )
	{
		const mrc_geometry geometry(
			make_header_of(4, 3, 6, 1, 0),
			mrc_single_section::image
		);

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{6, 3, 4} );
	}
}

TEST_CASE( "the axes of an MRC file are ordered by the axis of space each "
	"runs along",
	"[mrc_geometry]" )
{
	SECTION( "a volume whose columns run along Z is reported along Z, Y and X" )
	{
		// The axis correspondence of EMD-3001: its columns run along Z, its
		// rows along X and its sections along Y.
		const mrc_geometry geometry(
			with_axes(make_header_of(73, 43, 25, 72, 4), 3, 1, 2));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{73, 25, 43} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{1, 3139, 73} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
		REQUIRE( geometry.get_element_count() == 25 * 43 * 73 );
	}

	SECTION( "a single image swaps its two axes" )
	{
		const mrc_geometry geometry(
			with_axes(make_header_of(4, 3, 1, 1, 0), 2, 1, 3));

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{4, 3} );
		REQUIRE( strides_of(geometry) == std::vector<std::ptrdiff_t>{1, 4} );
	}

	SECTION( "the sections of a stack of images are no axis of space" )
	{
		const mrc_geometry geometry(
			with_axes(make_header_of(4, 3, 5, 1, 0), 2, 1, 3));

		REQUIRE( extents_of(geometry) == std::vector<std::size_t>{5, 4, 3} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{12, 1, 4} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "the volumes of a stack of volumes are no axis of space either" )
	{
		const mrc_geometry geometry(
			with_axes(make_header_of(4, 3, 12, 4, 401), 2, 3, 1));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{3, 3, 4, 4} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{48, 4, 1, 12} );
		REQUIRE( geometry.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "an axis correspondence that is no permutation is refused" )
	{
		REQUIRE_THROWS_AS(
			mrc_geometry(with_axes(make_header_of(4, 3, 5, 1, 0), 1, 1, 3)),
			image_format_error
		);
	}

	SECTION( "one that names an axis the format has none of is refused too" )
	{
		REQUIRE_THROWS_AS(
			mrc_geometry(with_axes(make_header_of(4, 3, 5, 1, 0), 1, 2, 4)),
			image_format_error
		);
	}

	SECTION( "an axis correspondence of zeros is read in order" )
	{
		const mrc_geometry geometry(
			with_axes(make_header_of(4, 3, 12, 4, 401), 0, 0, 0));

		REQUIRE( extents_of(geometry) ==
			std::vector<std::size_t>{3, 4, 3, 4} );
		REQUIRE( strides_of(geometry) ==
			std::vector<std::ptrdiff_t>{48, 12, 4, 1} );
	}
}

TEST_CASE( "an MRC file reports where and how much of it holds values",
	"[mrc_geometry]" )
{
	SECTION( "the values follow the main header" )
	{
		const mrc_geometry geometry(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( geometry.get_data_offset() == 1024 );
		REQUIRE( geometry.get_element_count() == 60 );
		REQUIRE( geometry.get_data_size() == 240 );
		REQUIRE( geometry.get_descriptor().get_data_type() ==
			numerical_type::float32 );
	}

	SECTION( "an extended header pushes them further" )
	{
		auto header = make_header_of(4, 3, 5, 1, 0);
		header.set_extended_header_size(160);

		const mrc_geometry geometry(header);

		REQUIRE( geometry.get_data_offset() == 1184 );
		REQUIRE( geometry.get_data_size() == 240 );
	}

	SECTION( "the data type resolves mode 0 through the IMOD stamp" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0, mrc_mode::int8);
		header.set_imod_stamp(1146047817);

		const mrc_geometry geometry(header);

		REQUIRE( geometry.get_descriptor().get_data_type() ==
			numerical_type::uint8 );
		REQUIRE( geometry.get_data_size() == 12 );
	}
}

TEST_CASE( "values that could not be addressed where they begin are refused",
	"[mrc_geometry]" )
{
	SECTION( "an extended header that misaligns them is refused" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0);
		header.set_extended_header_size(2);

		REQUIRE_THROWS_AS( mrc_geometry(header), image_format_error );
	}

	SECTION( "one that keeps them aligned is not" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0);
		header.set_extended_header_size(4);

		REQUIRE_NOTHROW( mrc_geometry(header) );
	}

	SECTION( "a narrower element tolerates a smaller multiple" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0, mrc_mode::int8);
		header.set_extended_header_size(3);

		REQUIRE_NOTHROW( mrc_geometry(header) );
	}
}
