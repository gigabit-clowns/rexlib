// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_geometry.hpp>

#include <em/image/formats/memory_mapping/image_file_layout.hpp>
#include <em/image/formats/mrc/mrc_header.hpp>
#include <em/image/formats/mrc/mrc_mode.hpp>

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/memory/byte_order.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>

#include <cstddef>
#include <cstdint>
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

std::vector<std::size_t> extents_of(const image_file_layout &layout)
{
	const auto extents = layout.get_descriptor().get_extents();
	return std::vector<std::size_t>(extents.begin(), extents.end());
}

std::vector<std::ptrdiff_t> strides_of(const image_file_layout &layout)
{
	const auto strides = layout.get_strides();
	return std::vector<std::ptrdiff_t>(strides.begin(), strides.end());
}

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

} // anonymous namespace

TEST_CASE( "the shape of an MRC file follows from its space group",
	"[mrc_geometry]" )
{
	SECTION( "one section and no space group is a single image" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 1, 1, 0));

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "several sections and no space group is a stack of images" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{5, 3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "a space group of one is a volume" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 5, 5, 1));

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{5, 3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a crystallographic space group is a volume too" )
	{
		const auto layout =
			derive_file_layout(make_header_of(73, 43, 25, 72, 4));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{25, 43, 73} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a space group above four hundred is a stack of volumes" )
	{
		const auto layout =
			derive_file_layout(make_header_of(4, 3, 12, 4, 401));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{3, 4, 3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "the last space group of the range is still a stack" )
	{
		const auto layout =
			derive_file_layout(make_header_of(4, 3, 12, 4, 630));

		REQUIRE( extents_of(layout).size() == 4 );
	}

	SECTION( "one past the range is a volume" )
	{
		const auto layout =
			derive_file_layout(make_header_of(4, 3, 12, 4, 631));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{12, 3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}
}

TEST_CASE( "a crystallographic file does not read its sampling as a depth",
	"[mrc_geometry]" )
{
	// The sampling of EMD-3001, which is unrelated to its section count.
	mrc_header header = make_header_of(73, 43, 25, 72, 4);
	header.set_column_sampling(40);
	header.set_row_sampling(12);

	const auto layout = derive_file_layout(header);

	REQUIRE( extents_of(layout) == std::vector<std::size_t>{25, 43, 73} );
	REQUIRE( layout.get_data_size() == 25 * 43 * 73 * sizeof(float) );
}

TEST_CASE( "the values of an MRC file are laid out contiguously",
	"[mrc_geometry]" )
{
	SECTION( "a single image counts by rows" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 1, 1, 0));

		REQUIRE( strides_of(layout) == std::vector<std::ptrdiff_t>{4, 1} );
	}

	SECTION( "a stack counts by sections and rows" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{12, 4, 1} );
	}

	SECTION( "a stack of volumes counts by volumes as well" )
	{
		const auto layout =
			derive_file_layout(make_header_of(4, 3, 12, 4, 401));

		REQUIRE( strides_of(layout) ==
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
		const auto layout = derive_file_layout(make_header_of(4, 3, 6, 1, 401));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{6, 1, 3, 4} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{12, 12, 4, 1} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a single volume" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 5, 5, 401));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{1, 5, 3, 4} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{60, 12, 4, 1} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "a single volume of a single section" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 1, 1, 401));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{1, 1, 3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
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
		const auto layout = derive_file_layout(header);

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{3, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "a stack of one image when told so" )
	{
		const auto layout =
			derive_file_layout(header, mrc_single_section::image_stack);

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{1, 3, 4} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{12, 4, 1} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "more than one section is a stack either way" )
	{
		const auto layout = derive_file_layout(
			make_header_of(4, 3, 6, 1, 0),
			mrc_single_section::image
		);

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{6, 3, 4} );
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
		const auto layout = derive_file_layout(
			with_axes(make_header_of(73, 43, 25, 72, 4), 3, 1, 2));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{73, 25, 43} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{1, 3139, 73} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
		REQUIRE( layout.get_data_size() == 25 * 43 * 73 * sizeof(float) );
	}

	SECTION( "a single image swaps its two axes" )
	{
		const auto layout = derive_file_layout(
			with_axes(make_header_of(4, 3, 1, 1, 0), 2, 1, 3));

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{4, 3} );
		REQUIRE( strides_of(layout) == std::vector<std::ptrdiff_t>{1, 4} );
	}

	SECTION( "the sections of a stack of images are no axis of space" )
	{
		const auto layout = derive_file_layout(
			with_axes(make_header_of(4, 3, 5, 1, 0), 2, 1, 3));

		REQUIRE( extents_of(layout) == std::vector<std::size_t>{5, 4, 3} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{12, 1, 4} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 2 );
	}

	SECTION( "the volumes of a stack of volumes are no axis of space either" )
	{
		const auto layout = derive_file_layout(
			with_axes(make_header_of(4, 3, 12, 4, 401), 2, 3, 1));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{3, 3, 4, 4} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{48, 4, 1, 12} );
		REQUIRE( layout.get_descriptor().get_core_rank() == 3 );
	}

	SECTION( "an axis correspondence that is no permutation is refused" )
	{
		REQUIRE_THROWS_AS(
			derive_file_layout(
				with_axes(make_header_of(4, 3, 5, 1, 0), 1, 1, 3)
			),
			image_format_error
		);
	}

	SECTION( "one that names an axis the format has none of is refused too" )
	{
		REQUIRE_THROWS_AS(
			derive_file_layout(
				with_axes(make_header_of(4, 3, 5, 1, 0), 1, 2, 4)
			),
			image_format_error
		);
	}

	SECTION( "an axis correspondence of zeros is read in order" )
	{
		const auto layout = derive_file_layout(
			with_axes(make_header_of(4, 3, 12, 4, 401), 0, 0, 0));

		REQUIRE( extents_of(layout) ==
			std::vector<std::size_t>{3, 4, 3, 4} );
		REQUIRE( strides_of(layout) ==
			std::vector<std::ptrdiff_t>{48, 12, 4, 1} );
	}
}

TEST_CASE( "an MRC file reports where and how much of it holds values",
	"[mrc_geometry]" )
{
	SECTION( "the values follow the main header" )
	{
		const auto layout = derive_file_layout(make_header_of(4, 3, 5, 1, 0));

		REQUIRE( layout.get_data_offset() == 1024 );
		REQUIRE( layout.get_data_size() == 240 );
		REQUIRE( layout.get_descriptor().get_data_type() ==
			numerical_type::float32 );
	}

	SECTION( "an extended header pushes them further" )
	{
		auto header = make_header_of(4, 3, 5, 1, 0);
		header.set_extended_header_size(160);

		const auto layout = derive_file_layout(header);

		REQUIRE( layout.get_data_offset() == 1184 );
		REQUIRE( layout.get_data_size() == 240 );
	}

	SECTION( "the data type resolves mode 0 through the IMOD stamp" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0, mrc_mode::int8);
		header.set_imod_stamp(1146047817);

		const auto layout = derive_file_layout(header);

		REQUIRE( layout.get_descriptor().get_data_type() ==
			numerical_type::uint8 );
		REQUIRE( layout.get_data_size() == 12 );
	}
}

TEST_CASE( "values that could not be addressed where they begin are refused",
	"[mrc_geometry]" )
{
	SECTION( "an extended header that misaligns them is refused" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0);
		header.set_extended_header_size(2);

		REQUIRE_THROWS_AS( derive_file_layout(header), image_format_error );
	}

	SECTION( "one that keeps them aligned is not" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0);
		header.set_extended_header_size(4);

		REQUIRE_NOTHROW( derive_file_layout(header) );
	}

	SECTION( "a narrower element tolerates a smaller multiple" )
	{
		auto header = make_header_of(4, 3, 1, 1, 0, mrc_mode::int8);
		header.set_extended_header_size(3);

		REQUIRE_NOTHROW( derive_file_layout(header) );
	}
}

TEST_CASE( "the layout of an MRC file is in the byte order of its header",
	"[mrc_geometry]" )
{
	auto header = make_header_of(4, 3, 1, 1, 0);

	SECTION( "little endian" )
	{
		header.set_byte_order(byte_order::little_endian);

		REQUIRE( derive_file_layout(header).get_byte_order() ==
			byte_order::little_endian );
	}

	SECTION( "big endian" )
	{
		header.set_byte_order(byte_order::big_endian);

		REQUIRE( derive_file_layout(header).get_byte_order() ==
			byte_order::big_endian );
	}
}

TEST_CASE( "a header is built from the shape a file is created with",
	"[mrc_geometry]" )
{
	const std::vector<std::size_t> image = {3, 4};
	const std::vector<std::size_t> stack = {5, 3, 4};
	const std::vector<std::size_t> volume_stack = {3, 4, 3, 4};

	SECTION( "a single image states one section and no space group" )
	{
		const auto header = make_header(make_descriptor(image, 2));

		REQUIRE( header.get_column_count() == 4 );
		REQUIRE( header.get_row_count() == 3 );
		REQUIRE( header.get_section_count() == 1 );
		REQUIRE( header.get_section_sampling() == 1 );
		REQUIRE( header.get_space_group() == 0 );
	}

	SECTION( "a stack of images states a sampling of one" )
	{
		const auto header = make_header(make_descriptor(stack, 2));

		REQUIRE( header.get_section_count() == 5 );
		REQUIRE( header.get_section_sampling() == 1 );
		REQUIRE( header.get_space_group() == 0 );
	}

	SECTION( "a volume states its depth as its sampling" )
	{
		const auto header = make_header(make_descriptor(stack, 3));

		REQUIRE( header.get_section_count() == 5 );
		REQUIRE( header.get_section_sampling() == 5 );
		REQUIRE( header.get_space_group() == 1 );
	}

	SECTION( "a stack of volumes divides its sections between two axes" )
	{
		const auto header = make_header(make_descriptor(volume_stack, 3));

		REQUIRE( header.get_section_count() == 12 );
		REQUIRE( header.get_section_sampling() == 4 );
		REQUIRE( header.get_space_group() == 401 );
	}

	SECTION( "a stack of one volume is still a stack of volumes" )
	{
		const std::vector<std::size_t> single = {1, 3, 3, 4};
		const auto header = make_header(make_descriptor(single, 3));

		REQUIRE( header.get_section_count() == 3 );
		REQUIRE( header.get_section_sampling() == 3 );
		REQUIRE( header.get_space_group() == 401 );
	}

	SECTION( "volumes one section deep are still a stack of volumes" )
	{
		const std::vector<std::size_t> flat = {3, 1, 3, 4};
		const auto header = make_header(make_descriptor(flat, 3));

		REQUIRE( header.get_section_count() == 3 );
		REQUIRE( header.get_section_sampling() == 1 );
		REQUIRE( header.get_space_group() == 401 );
	}

	// MRC2014 has no other header for a stack of one image than that of a
	// single image, so which one the file holds is up to how it is read.
	SECTION( "a stack of one image gets the header of a single image" )
	{
		const std::vector<std::size_t> single = {1, 3, 4};
		const auto header = make_header(make_descriptor(single, 2));

		REQUIRE( header.get_section_count() == 1 );
		REQUIRE( header.get_section_sampling() == 1 );
		REQUIRE( header.get_space_group() == 0 );
	}

	SECTION( "what it does not derive is what a new file carries" )
	{
		const auto header = make_header(make_descriptor(image, 2));

		REQUIRE( header.get_column_axis() == 1 );
		REQUIRE( header.get_row_axis() == 2 );
		REQUIRE( header.get_section_axis() == 3 );
		REQUIRE( header.get_version() == 20141 );
		REQUIRE( header.get_cell_angles()[0] == 90.0F );
		REQUIRE( header.get_byte_order() == get_system_byte_order() );
	}

	SECTION( "statistics that were not computed carry their sentinels" )
	{
		const auto header = make_header(make_descriptor(image, 2));

		REQUIRE( header.get_data_min() == 0.0F );
		REQUIRE( header.get_data_max() == -1.0F );
		REQUIRE( header.get_data_mean() == -2.0F );
		REQUIRE( header.get_data_rms() == -1.0F );
	}

	SECTION( "unsigned bytes are stamped as such" )
	{
		const auto header =
			make_header(make_descriptor(image, 2, numerical_type::uint8));

		REQUIRE( header.get_mode() == mrc_mode::int8 );
		REQUIRE( header.get_imod_stamp() == 1146047817 );
		REQUIRE_FALSE( holds_signed_bytes(header) );
	}

	SECTION( "signed bytes are not" )
	{
		const auto header =
			make_header(make_descriptor(image, 2, numerical_type::int8));

		REQUIRE( header.get_mode() == mrc_mode::int8 );
		REQUIRE( header.get_imod_stamp() == 0 );
		REQUIRE( holds_signed_bytes(header) );
	}

	SECTION( "it is signed with the library that built it" )
	{
		const auto header = make_header(make_descriptor(image, 2));

		const auto labels = header.get_labels();

		REQUIRE( labels.size() == 1 );
		REQUIRE( labels[0].compare(0, 17, "Created by rexlib") == 0 );
	}
}

TEST_CASE( "a shape the MRC format cannot hold builds no header",
	"[mrc_geometry]" )
{
	const std::vector<std::size_t> image = {3, 4};
	const std::vector<std::size_t> stack = {5, 3, 4};
	const std::vector<std::size_t> volume_stack = {3, 4, 3, 4};
	const std::vector<std::size_t> line = {4};
	const std::vector<std::size_t> too_deep = {2, 3, 4, 3, 4};

	SECTION( "a rank of one is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(line, 1)),
			unsupported_operation_error
		);
	}

	// A volume of one section is not a stack of one of anything: the format
	// states it as a section count of one, which is what it reads back as.
	SECTION( "a volume of a single section is not refused" )
	{
		const std::vector<std::size_t> flat = {1, 3, 4};

		REQUIRE_NOTHROW(
			make_header(make_descriptor(flat, 3))
		);
	}

	SECTION( "a rank above four is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(too_deep, 3)),
			unsupported_operation_error
		);
	}

	SECTION( "a stack of images of images is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(volume_stack, 2)),
			unsupported_operation_error
		);
	}

	SECTION( "a data type the format has no mode for is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(stack, 2, numerical_type::float64)),
			unsupported_operation_error
		);
	}
}

TEST_CASE( "a header built from a shape resolves back into that shape",
	"[mrc_geometry]" )
{
	SECTION( "every shape the format holds" )
	{
		// A stack of one volume and volumes one section deep are the two
		// stacks of volumes the space group alone tells from a volume.
		const std::vector<std::vector<std::size_t>> shapes = {
			{3, 4}, {5, 3, 4}, {5, 3, 4}, {3, 4, 3, 4}, {1, 2, 3, 4},
			{2, 1, 3, 4}
		};
		const std::size_t core_ranks[] = {2, 2, 3, 3, 3, 3};

		for (std::size_t i = 0; i < shapes.size(); ++i)
		{
			const auto descriptor =
				make_descriptor(shapes[i], core_ranks[i]);

			REQUIRE(
				derive_file_layout(make_header(descriptor)).get_descriptor() ==
				descriptor
			);
		}
	}

	SECTION( "a stack of one image where it is read as a stack" )
	{
		const std::vector<std::size_t> single = {1, 3, 4};
		const auto descriptor = make_descriptor(single, 2);
		const auto layout = derive_file_layout(
			make_header(descriptor),
			mrc_single_section::image_stack
		);

		REQUIRE( layout.get_descriptor() == descriptor );
	}

	SECTION( "every data type a mode holds" )
	{
		// Unsigned bytes share a mode with signed ones, and are told apart
		// by the stamp the header carries.
		const std::vector<std::size_t> image = {3, 4};
		const numerical_type data_types[] = {
			numerical_type::int8,
			numerical_type::uint8,
			numerical_type::int16,
			numerical_type::uint16,
			numerical_type::float32
		};

		for (const auto data_type : data_types)
		{
			const auto descriptor = make_descriptor(image, 2, data_type);

			REQUIRE(
				derive_file_layout(make_header(descriptor)).get_descriptor() ==
				descriptor
			);
		}
	}

	SECTION( "the values begin right after the header, in host order" )
	{
		const std::vector<std::size_t> image = {3, 4};
		const auto layout =
			derive_file_layout(make_header(make_descriptor(image, 2)));

		REQUIRE( layout.get_data_offset() == 1024 );
		REQUIRE( layout.get_byte_order() == get_system_byte_order() );
	}
}
