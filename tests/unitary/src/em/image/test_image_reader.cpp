// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_reader.hpp>

#include "fake/fake_image_reader.hpp"
#include "mock/mock_image_reader.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// The file the fake serves: a stack of 4 images of 3 rows by 5 columns,
// holding the values 0, 1, 2, ... in storage order.
const std::vector<std::size_t> file_extents = {4, 3, 5};

std::unique_ptr<fake_image_reader> make_reader()
{
	const std::vector<std::size_t> granularity = {1, 1, 1};
	return std::unique_ptr<fake_image_reader>(new fake_image_reader(
		make_span(file_extents),
		image_access_traits(
			make_span(granularity),
			image_access_flags({
				image_access_flag_bits::ordered_offsets,
				image_access_flag_bits::concurrent_read
			}),
			numerical_type::int16
		),
		image_metadata()
	));
}

array make_destination(
	span<const std::size_t> extents,
	numerical_type data_type
)
{
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(extents),
		data_type
	);
	const auto size = compute_storage_requirement(descriptor);
	const auto storage = std::make_shared<host_buffer>(
		size,
		alignof(std::max_align_t)
	);
	// Zeroed so that a test may assert that a region was not written to.
	std::memset(storage->get_host_ptr(), 0, size);
	return array(storage, descriptor);
}

const std::int16_t* int16_data(const array &target)
{
	return static_cast<const std::int16_t*>(
		target.get_storage()->get_host_ptr()
	);
}

const float* float_data(const array &target)
{
	return static_cast<const float*>(target.get_storage()->get_host_ptr());
}

// Build a batch of whole stack elements: file element sources[i] lands in
// slot i of the destination.
image_region_batch make_element_batch(
	span<const std::size_t> extents,
	const std::vector<std::size_t> &sources
)
{
	image_region_batch regions;
	regions.reset(extents, file_extents.size());
	regions.reserve(sources.size());

	for (std::size_t slot = 0; slot < sources.size(); ++slot)
	{
		const std::size_t file_offset[3] = {sources[slot], 0, 0};
		const std::size_t array_offset[3] = {slot, 0, 0};
		regions.add(
			make_span(file_offset, 3),
			make_span(array_offset, 3)
		);
	}

	return regions;
}

} // namespace

TEST_CASE( "every read pattern is one region", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "an element of a stack is a region with a leading extent of 1" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t file_offset[3] = {2, 0, 0};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

		reader->read(array_ref(destination), regions);

		// Element 2 starts at 2 * 3 * 5 = 30.
		const auto *values = int16_data(destination);
		REQUIRE( values[0] == 30 );
		REQUIRE( values[14] == 44 );
	}

	SECTION( "a patch is a region that starts inside a plane" )
	{
		const std::vector<std::size_t> extents = {1, 2, 2};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t file_offset[3] = {1, 1, 3};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

		reader->read(array_ref(destination), regions);

		// Plane 1 starts at 15; row 1 adds 5; column 3 adds 3.
		const auto *values = int16_data(destination);
		REQUIRE( values[0] == 23 );
		REQUIRE( values[1] == 24 );
		REQUIRE( values[2] == 28 );
		REQUIRE( values[3] == 29 );
	}

	SECTION( "the whole file is the region covering everything" )
	{
		auto destination = make_destination(
			make_span(file_extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(file_extents), 3);
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));

		reader->read(array_ref(destination), regions);

		const auto *values = int16_data(destination);
		for (std::size_t i = 0; i < 60; ++i)
		{
			REQUIRE( values[i] == static_cast<std::int16_t>(i) );
		}
	}

	SECTION( "an empty batch reads nothing and succeeds" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);

		REQUIRE_NOTHROW( reader->read(array_ref(destination), regions) );
	}
}

TEST_CASE( "one read fills a whole batch", "[image_reader]" )
{
	const auto reader = make_reader();
	const std::vector<std::size_t> batch_extents = {3, 3, 5};
	const std::vector<std::size_t> element_extents = {1, 3, 5};

	SECTION( "slot i receives request i, in whatever order they are read" )
	{
		auto batch = make_destination(
			make_span(batch_extents),
			numerical_type::float32
		);

		// Deliberately not in increasing file order, so that a reader
		// that sorted the regions and forgot to keep the slots with them
		// would be caught.
		const std::vector<std::size_t> sources = {3, 0, 2};
		const auto regions = make_element_batch(
			make_span(element_extents),
			sources
		);

		reader->read(array_ref(batch), regions);

		const auto *values = float_data(batch);
		REQUIRE( values[0] == 45.0f );
		REQUIRE( values[15] == 0.0f );
		REQUIRE( values[30] == 30.0f );
	}

	SECTION( "the same element may be requested into several slots" )
	{
		auto batch = make_destination(
			make_span(batch_extents),
			numerical_type::int16
		);

		const std::vector<std::size_t> sources = {1, 1, 1};
		const auto regions = make_element_batch(
			make_span(element_extents),
			sources
		);

		reader->read(array_ref(batch), regions);

		const auto *values = int16_data(batch);
		REQUIRE( values[0] == 15 );
		REQUIRE( values[15] == 15 );
		REQUIRE( values[30] == 15 );
	}
}

TEST_CASE( "a read converts to the destination data type", "[image_reader]" )
{
	const auto reader = make_reader();
	const std::vector<std::size_t> extents = {1, 1, 4};

	const auto make_regions = [&] ()
	{
		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t file_offset[3] = {0, 1, 0};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 3), make_span(array_offset, 3));
		return regions;
	};

	SECTION( "the preferred data type is copied unchanged" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		reader->read(array_ref(destination), make_regions());

		const auto *values = int16_data(destination);
		REQUIRE( values[0] == 5 );
		REQUIRE( values[3] == 8 );
	}

	SECTION( "another data type is converted preserving the value" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::float32
		);
		reader->read(array_ref(destination), make_regions());

		const auto *values = float_data(destination);
		REQUIRE( values[0] == 5.0f );
		REQUIRE( values[3] == 8.0f );
	}

	SECTION( "a data type that can not be produced is refused" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::complex_float32
		);

		REQUIRE_THROWS_AS(
			reader->read(array_ref(destination), make_regions()),
			invalid_operation_error
		);
	}
}

TEST_CASE( "a read fills a strided destination", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "slots of a batch are reached through the destination strides" )
	{
		// The batch is wider than the elements written into it, so the
		// regions land on a strided part of it rather than contiguously.
		const std::vector<std::size_t> batch_extents = {2, 3, 10};
		auto batch = make_destination(
			make_span(batch_extents),
			numerical_type::int16
		);

		const std::vector<std::size_t> element_extents = {1, 3, 5};
		image_region_batch regions;
		regions.reset(make_span(element_extents), 3);
		const std::size_t first_file[3] = {0, 0, 0};
		const std::size_t first_array[3] = {0, 0, 0};
		const std::size_t second_file[3] = {1, 0, 0};
		const std::size_t second_array[3] = {1, 0, 5};
		regions.add(make_span(first_file, 3), make_span(first_array, 3));
		regions.add(make_span(second_file, 3), make_span(second_array, 3));

		reader->read(array_ref(batch), regions);

		const auto *values = int16_data(batch);
		REQUIRE( values[0] == 0 );
		REQUIRE( values[4] == 4 );
		REQUIRE( values[5] == 0 );
		REQUIRE( values[35] == 15 );
		REQUIRE( values[30] == 0 );
	}
}

TEST_CASE( "a read refuses a region it can not serve", "[image_reader]" )
{
	const auto reader = make_reader();
	const std::vector<std::size_t> extents = {1, 3, 5};

	SECTION( "a region reaching past the file is out of range" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t file_offset[3] = {4, 0, 0};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

		REQUIRE_THROWS_AS(
			reader->read(array_ref(destination), regions),
			std::out_of_range
		);
	}

	SECTION( "a region overhanging the destination is out of range" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t file_offset[3] = {0, 0, 0};
		const std::size_t array_offset[3] = {1, 0, 0};
		regions.add(make_span(file_offset, 3), make_span(array_offset, 3));

		REQUIRE_THROWS_AS(
			reader->read(array_ref(destination), regions),
			std::out_of_range
		);
	}

	SECTION( "a batch of the wrong file rank is refused" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 2);
		const std::size_t file_offset[2] = {0, 0};
		const std::size_t array_offset[3] = {0, 0, 0};
		regions.add(make_span(file_offset, 2), make_span(array_offset, 3));

		REQUIRE_THROWS_AS(
			reader->read(array_ref(destination), regions),
			std::invalid_argument
		);
	}

	SECTION( "extents of the wrong rank are refused, never adjusted" )
	{
		const std::vector<std::size_t> flat_extents = {3, 5};
		auto destination = make_destination(
			make_span(flat_extents),
			numerical_type::int16
		);

		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));

		REQUIRE_THROWS_AS(
			reader->read(array_ref(destination), regions),
			std::invalid_argument
		);
	}

	SECTION( "an uninitialized destination is refused" )
	{
		image_region_batch regions;
		regions.reset(make_span(extents), 3);
		const std::size_t origin[3] = {0, 0, 0};
		regions.add(make_span(origin, 3), make_span(origin, 3));

		REQUIRE_THROWS_AS(
			reader->read(array_ref(), regions),
			std::invalid_argument
		);
	}
}

TEST_CASE( "a reader reports what it was opened with", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "the descriptor names the file" )
	{
		std::vector<std::size_t> extents;
		reader->get_descriptor().get_layout().get_extents(extents);

		REQUIRE( extents == file_extents );
		REQUIRE( reader->get_descriptor().get_data_type() ==
			numerical_type::int16 );
	}

	SECTION( "the traits state what a read costs" )
	{
		std::vector<std::size_t> granularity;
		reader->get_access_traits().get_granularity(granularity);

		REQUIRE( granularity == std::vector<std::size_t>{1, 1, 1} );
		REQUIRE( reader->get_access_traits().get_flags().contains(
			image_access_flag_bits::concurrent_read) );
		REQUIRE( reader->get_access_traits().get_preferred_data_type() ==
			numerical_type::int16 );
	}
}

TEST_CASE( "image_reader is mockable", "[image_reader]" )
{
	mock_image_reader reader;
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(file_extents)),
		numerical_type::float32
	);

	ALLOW_CALL(reader, get_descriptor()).RETURN(descriptor);
	REQUIRE_CALL(reader, read(ANY(array_ref),
		ANY(const image_region_batch&)));

	const image_reader &interface = reader;
	std::vector<std::size_t> extents;
	interface.get_descriptor().get_layout().get_extents(extents);

	REQUIRE( extents == file_extents );

	const image_region_batch regions;
	interface.read(array_ref(), regions);
}
