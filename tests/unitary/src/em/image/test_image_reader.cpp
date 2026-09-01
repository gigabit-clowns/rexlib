// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_reader.hpp>

#include "fake/fake_image_reader.hpp"
#include "mock/mock_image_reader.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/layout/slice.hpp>
#include <rexlib/core/layout/subscript_tags.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/functional/view.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// The dataset the fake serves: a stack of 4 images of 3 rows by 5 columns,
// holding the values 0, 1, 2, ... in storage order.
const std::vector<std::size_t> dataset_extents = {4, 3, 5};

std::unique_ptr<fake_image_reader> make_reader()
{
	const std::vector<std::size_t> granularity = {1, 1, 1};
	return std::unique_ptr<fake_image_reader>(new fake_image_reader(
		make_span(dataset_extents),
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
	const auto storage = std::make_shared<host_buffer>(
		compute_storage_requirement(descriptor),
		alignof(std::max_align_t)
	);
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
		const std::vector<std::size_t> offset = {2, 0, 0};
		reader->read_region(make_span(offset), array_ref(destination));

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
		const std::vector<std::size_t> offset = {1, 1, 3};
		reader->read_region(make_span(offset), array_ref(destination));

		// Plane 1 starts at 15; row 1 adds 5; column 3 adds 3.
		const auto *values = int16_data(destination);
		REQUIRE( values[0] == 23 );
		REQUIRE( values[1] == 24 );
		REQUIRE( values[2] == 28 );
		REQUIRE( values[3] == 29 );
	}

	SECTION( "the whole dataset is the region covering everything" )
	{
		auto destination = make_destination(
			make_span(dataset_extents),
			numerical_type::int16
		);
		const std::vector<std::size_t> offset = {0, 0, 0};
		reader->read_region(make_span(offset), array_ref(destination));

		const auto *values = int16_data(destination);
		for (std::size_t i = 0; i < 60; ++i)
		{
			REQUIRE( values[i] == static_cast<std::int16_t>(i) );
		}
	}
}

TEST_CASE( "a read converts to the destination data type", "[image_reader]" )
{
	const auto reader = make_reader();
	const std::vector<std::size_t> extents = {1, 1, 4};
	const std::vector<std::size_t> offset = {0, 1, 0};

	SECTION( "the preferred data type is copied unchanged" )
	{
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		reader->read_region(make_span(offset), array_ref(destination));

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
		reader->read_region(make_span(offset), array_ref(destination));

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
			reader->read_region(make_span(offset), array_ref(destination)),
			invalid_operation_error
		);
	}
}

TEST_CASE( "a read fills a strided destination", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "one slot of a batch receives one element" )
	{
		// A batch of three slots, each shaped like one element of the stack.
		const std::vector<std::size_t> batch_extents = {3, 1, 3, 5};
		auto batch = make_destination(
			make_span(batch_extents),
			numerical_type::float32
		);

		const std::vector<std::size_t> requests = {0, 3, 1};
		for (std::size_t slot = 0; slot < requests.size(); ++slot)
		{
			const std::vector<dynamic_subscript> subscripts = {
				static_cast<std::ptrdiff_t>(slot), ellipsis()
			};
			auto view = subarray(batch, make_span(subscripts));

			const std::vector<std::size_t> offset = {requests[slot], 0, 0};
			reader->read_region(make_span(offset), array_ref(view));
		}

		// Slot i holds request i, whichever order they were issued in.
		const auto *values = float_data(batch);
		REQUIRE( values[0] == 0.0f );
		REQUIRE( values[15] == 45.0f );
		REQUIRE( values[30] == 15.0f );
	}

	SECTION( "a destination with a step is filled through its strides" )
	{
		const std::vector<std::size_t> extents = {1, 3, 10};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);

		const std::vector<dynamic_subscript> subscripts = {
			all(), all(), make_slice(0, 5, 2)
		};
		auto view = subarray(destination, make_span(subscripts));

		const std::vector<std::size_t> offset = {0, 0, 0};
		reader->read_region(make_span(offset), array_ref(view));

		const auto *values = int16_data(destination);
		REQUIRE( values[0] == 0 );
		REQUIRE( values[2] == 1 );
		REQUIRE( values[4] == 2 );
		REQUIRE( values[10] == 5 );
	}
}

TEST_CASE( "a read refuses a region it can not serve", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "a region reaching past the dataset is out of range" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		const std::vector<std::size_t> offset = {4, 0, 0};

		REQUIRE_THROWS_AS(
			reader->read_region(make_span(offset), array_ref(destination)),
			std::out_of_range
		);
	}

	SECTION( "a region overhanging an inner axis is out of range" )
	{
		const std::vector<std::size_t> extents = {1, 2, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		const std::vector<std::size_t> offset = {0, 2, 0};

		REQUIRE_THROWS_AS(
			reader->read_region(make_span(offset), array_ref(destination)),
			std::out_of_range
		);
	}

	SECTION( "an offset of the wrong rank is refused" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		const std::vector<std::size_t> offset = {0, 0};

		REQUIRE_THROWS_AS(
			reader->read_region(make_span(offset), array_ref(destination)),
			std::invalid_argument
		);
	}

	SECTION( "a destination of the wrong rank is refused, never adjusted" )
	{
		const std::vector<std::size_t> extents = {3, 5};
		auto destination = make_destination(
			make_span(extents),
			numerical_type::int16
		);
		const std::vector<std::size_t> offset = {0, 0, 0};

		REQUIRE_THROWS_AS(
			reader->read_region(make_span(offset), array_ref(destination)),
			std::invalid_argument
		);
	}

	SECTION( "an uninitialized destination is refused" )
	{
		array destination;
		const std::vector<std::size_t> offset = {0, 0, 0};

		REQUIRE_THROWS_AS(
			reader->read_region(make_span(offset), array_ref(destination)),
			std::invalid_argument
		);
	}
}

TEST_CASE( "a reader reports what it was opened with", "[image_reader]" )
{
	const auto reader = make_reader();

	SECTION( "the descriptor names the dataset" )
	{
		std::vector<std::size_t> extents;
		reader->get_descriptor().get_layout().get_extents(extents);

		REQUIRE( extents == dataset_extents );
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
		strided_layout::make_contiguous_layout(make_span(dataset_extents)),
		numerical_type::float32
	);

	ALLOW_CALL(reader, get_descriptor()).RETURN(descriptor);
	REQUIRE_CALL(reader, read_region(ANY(span<const std::size_t>),
		ANY(array_ref)));

	const image_reader &interface = reader;
	std::vector<std::size_t> extents;
	interface.get_descriptor().get_layout().get_extents(extents);

	REQUIRE( extents == dataset_extents );

	const std::vector<std::size_t> offset = {0, 0, 0};
	interface.read_region(make_span(offset), array_ref());
}
