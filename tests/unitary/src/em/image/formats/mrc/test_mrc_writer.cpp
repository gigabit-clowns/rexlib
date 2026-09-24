// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_writer.hpp>

#include <em/image/formats/mrc/mrc_geometry.hpp>
#include <em/image/formats/mrc/mrc_header.hpp>
#include <em/image/formats/mrc/mrc_reader.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>

#include "../../fixtures/scoped_path.hpp"

#include <boost/filesystem/operations.hpp>

#include <cstring>
#include <fstream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

std::size_t element_count(const std::vector<std::size_t> &extents)
{
	return std::accumulate(
		extents.cbegin(),
		extents.cend(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

array make_host_array(
	const std::vector<std::size_t> &extents,
	const std::vector<float> &values
)
{
	auto storage = std::make_shared<host_buffer>(
		element_count(extents) * sizeof(float),
		alignof(float)
	);
	std::memcpy(
		storage->get_host_ptr(),
		values.data(),
		values.size() * sizeof(float)
	);

	return array(
		std::move(storage),
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type::float32
		)
	);
}

mrc_header header_of(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	std::vector<rexlib::byte> raw(header_size);
	input.read(reinterpret_cast<char*>(raw.data()), header_size);

	return parse_header(make_span(raw.data(), raw.size()));
}

std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i) + 0.5F;
	}

	return values;
}

image_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	std::size_t core_rank,
	numerical_type data_type = numerical_type::float32
)
{
	return image_descriptor(make_span(extents), core_rank, data_type);
}

image_transfer_plan whole_of(const std::vector<std::size_t> &extents)
{
	image_transfer_plan regions(
		make_span(extents), extents.size(), extents.size());
	regions.add(
		make_span(std::vector<std::size_t>(extents.size(), 0)),
		make_span(std::vector<std::size_t>(extents.size(), 0))
	);

	return regions;
}

std::vector<float> read_back(
	const std::string &path,
	const std::vector<std::size_t> &extents
)
{
	const mrc_reader reader(path);

	auto destination = make_host_array(
		extents, std::vector<float>(element_count(extents), -1.0F));
	reader.read(array_ref(destination), whole_of(extents));

	const auto *data = static_cast<const float*>(
		destination.get_storage()->get_host_ptr());

	return std::vector<float>(data, data + element_count(extents));
}

} // anonymous namespace

TEST_CASE( "a header is built from the shape a file is created with",
	"[mrc_writer]" )
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
	"[mrc_writer]" )
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
			invalid_operation_error
		);
	}

	// The format states the count of images or volumes of a stack as a
	// section count, and the depth of a volume beside it, which leaves it no
	// way to say that either of them is one: such a file states the shape
	// without that axis and reads back as it.
	SECTION( "a stack of one image is refused" )
	{
		const std::vector<std::size_t> single = {1, 3, 4};

		REQUIRE_THROWS_AS(
			make_header(make_descriptor(single, 2)),
			invalid_operation_error
		);
	}

	SECTION( "a stack of one volume is refused" )
	{
		const std::vector<std::size_t> single = {1, 3, 3, 4};

		REQUIRE_THROWS_AS(
			make_header(make_descriptor(single, 3)),
			invalid_operation_error
		);
	}

	SECTION( "a stack of volumes one section thick is refused" )
	{
		const std::vector<std::size_t> flat = {3, 1, 3, 4};

		REQUIRE_THROWS_AS(
			make_header(make_descriptor(flat, 3)),
			invalid_operation_error
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
			invalid_operation_error
		);
	}

	SECTION( "a stack of images of images is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(volume_stack, 2)),
			invalid_operation_error
		);
	}

	SECTION( "a data type the format has no mode for is refused" )
	{
		REQUIRE_THROWS_AS(
			make_header(make_descriptor(stack, 2, numerical_type::float64)),
			invalid_operation_error
		);
	}
}

TEST_CASE( "a header built from a shape reports that shape when parsed",
	"[mrc_writer]" )
{
	const std::vector<std::vector<std::size_t>> shapes = {
		{3, 4}, {5, 3, 4}, {5, 3, 4}, {3, 4, 3, 4}
	};
	const std::size_t core_ranks[] = {2, 2, 3, 3};

	for (std::size_t i = 0; i < shapes.size(); ++i)
	{
		const auto header =
			make_header(make_descriptor(shapes[i], core_ranks[i]));
		const mrc_geometry geometry(header);

		REQUIRE( geometry.get_descriptor() ==
			make_descriptor(shapes[i], core_ranks[i]) );
	}
}

TEST_CASE( "an MRC file is created with the shape it is opened over",
	"[mrc_writer]" )
{
	const scoped_path path("writer_created.mrc");

	SECTION( "a writer reports what it was created over" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		mrc_writer writer(path.get(), make_descriptor(extents, 2));

		REQUIRE( writer.get_descriptor() == make_descriptor(extents, 2) );
	}

	SECTION( "the file is laid out in full before anything is written" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		{
			const mrc_writer writer(path.get(), make_descriptor(extents, 2));
		}

		REQUIRE( boost::filesystem::file_size(path.get()) ==
			1024 + 24 * sizeof(float) );
	}

	SECTION( "the file records the library that wrote it" )
	{
		const std::vector<std::size_t> extents = {3, 4};
		{
			const mrc_writer writer(path.get(), make_descriptor(extents, 2));
		}

		// The header owns the labels the span refers to, so it has to
		// outlive it.
		const auto header = header_of(path.get());
		const auto labels = header.get_labels();

		REQUIRE( labels.size() == 1 );
		REQUIRE( labels[0].compare(0, 17, "Created by rexlib") == 0 );
	}

	SECTION( "a shape the format has no file for is refused" )
	{
		const std::vector<std::size_t> line = {4};

		REQUIRE_THROWS_AS(
			mrc_writer(path.get(), make_descriptor(line, 1)),
			invalid_operation_error
		);
	}

	SECTION( "a data type no mode holds is refused" )
	{
		const std::vector<std::size_t> extents = {2, 3};

		REQUIRE_THROWS_AS(
			mrc_writer(
				path.get(),
				make_descriptor(extents, 2, numerical_type::float64)
			),
			invalid_operation_error
		);
	}
}

TEST_CASE( "what is written to an MRC file is what is read back",
	"[mrc_writer]" )
{
	const scoped_path path("writer_round_trip.mrc");

	SECTION( "a single image round-trips" )
	{
		const std::vector<std::size_t> extents = {3, 4};
		const auto values = counting(12);

		{
			mrc_writer writer(path.get(), make_descriptor(extents, 2));
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "a stack of images round-trips, and keeps its core rank" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		const auto values = counting(24);

		{
			mrc_writer writer(path.get(), make_descriptor(extents, 2));
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor().get_core_rank() == 2 );
		REQUIRE( reader.get_descriptor().get_extents()[0] == 2 );
		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "a volume round-trips, and keeps its core rank" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};
		const auto values = counting(24);

		{
			mrc_writer writer(path.get(), make_descriptor(extents, 3));
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor().get_core_rank() == 3 );
		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "a stack of volumes round-trips, and keeps its four axes" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4, 5};
		const auto values = counting(120);

		{
			mrc_writer writer(path.get(), make_descriptor(extents, 3));
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor().get_extents().size() == 4 );
		REQUIRE( reader.get_descriptor().get_core_rank() == 3 );
		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "a narrower file keeps what its type can hold" )
	{
		const std::vector<std::size_t> extents = {2, 2};
		const std::vector<float> values = {-2.0F, -1.0F, 0.0F, 300.0F};

		{
			mrc_writer writer(
				path.get(),
				make_descriptor(extents, 2, numerical_type::int16)
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor().get_data_type() ==
			numerical_type::int16 );
		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "unsigned bytes survive the mode they share with signed ones" )
	{
		const std::vector<std::size_t> extents = {2, 2};
		const std::vector<float> values = {0.0F, 1.0F, 200.0F, 255.0F};

		{
			mrc_writer writer(
				path.get(),
				make_descriptor(extents, 2, numerical_type::uint8)
			);
			const auto source = make_host_array(extents, values);
			writer.write(const_array_ref(source), whole_of(extents));
			writer.flush();
		}

		const mrc_reader reader(path.get());

		REQUIRE( reader.get_descriptor().get_data_type() ==
			numerical_type::uint8 );
		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "regions written one at a time fill the file" )
	{
		const std::vector<std::size_t> extents = {3, 2, 2};
		const std::vector<std::size_t> plane = {2, 2};
		const auto values = counting(12);

		{
			mrc_writer writer(path.get(), make_descriptor(extents, 2));
			const auto source = make_host_array(extents, values);

			for (std::size_t i = 0; i < 3; ++i)
			{
				image_transfer_plan regions(make_span(plane), 3, 3);
				regions.add(
					make_span(std::vector<std::size_t>{i, 0, 0}),
					make_span(std::vector<std::size_t>{i, 0, 0})
				);
				writer.write(const_array_ref(source), regions);
			}

			writer.flush();
		}

		REQUIRE( read_back(path.get(), extents) == values );
	}

	SECTION( "an uninitialized source is refused" )
	{
		const std::vector<std::size_t> extents = {2, 2};
		mrc_writer writer(path.get(), make_descriptor(extents, 2));

		REQUIRE_THROWS_AS(
			writer.write(const_array_ref(), whole_of(extents)),
			std::invalid_argument
		);
	}
}
