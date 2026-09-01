// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_writer.hpp>

#include "fake/fake_image_writer.hpp"
#include "mock/mock_image_writer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> dataset_extents = {4, 3, 5};

array make_source(
	span<const std::size_t> extents,
	numerical_type data_type,
	float first_value
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
	array result(storage, descriptor);

	const auto count = descriptor.get_layout().compute_element_count();
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto value = first_value + static_cast<float>(i);
		if (data_type == numerical_type::int16)
		{
			static_cast<std::int16_t*>(storage->get_host_ptr())[i] =
				static_cast<std::int16_t>(value);
		}
		else
		{
			static_cast<float*>(storage->get_host_ptr())[i] = value;
		}
	}

	return result;
}

image_region_batch make_regions(
	span<const std::size_t> extents,
	span<const std::size_t> dataset_offset,
	span<const std::size_t> array_offset
)
{
	image_region_batch regions;
	regions.reset(extents, dataset_extents.size());
	regions.add(dataset_offset, array_offset);
	return regions;
}

} // namespace

TEST_CASE( "a write places one region of the dataset", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));
	const std::vector<std::size_t> extents = {1, 3, 5};
	const std::size_t origin[3] = {0, 0, 0};

	SECTION( "an element of a stack lands where it belongs" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			100.0f
		);
		const std::size_t dataset_offset[3] = {2, 0, 0};
		writer.write(
			const_array_ref(source),
			make_regions(
				make_span(extents),
				make_span(dataset_offset, 3),
				make_span(origin, 3)
			)
		);

		REQUIRE( writer.get_element(30) == 100 );
		REQUIRE( writer.get_element(44) == 114 );
		REQUIRE( writer.get_element(29) == 0 );
	}

	SECTION( "a region that is never written keeps the laid out value" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			1.0f
		);
		writer.write(
			const_array_ref(source),
			make_regions(
				make_span(extents),
				make_span(origin, 3),
				make_span(origin, 3)
			)
		);

		REQUIRE( writer.get_element(15) == 0 );
		REQUIRE( writer.get_element(59) == 0 );
	}

	SECTION( "an empty batch writes nothing and succeeds" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			1.0f
		);
		image_region_batch regions;
		regions.reset(make_span(extents), 3);

		REQUIRE_NOTHROW( writer.write(const_array_ref(source), regions) );
		REQUIRE( writer.get_element(0) == 0 );
	}
}

TEST_CASE( "one write drains a whole batch", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	// A batch of three elements written out in one call, deliberately not in
	// increasing dataset order.
	const std::vector<std::size_t> batch_extents = {3, 3, 5};
	auto batch = make_source(
		make_span(batch_extents),
		numerical_type::int16,
		0.0f
	);

	const std::vector<std::size_t> element_extents = {1, 3, 5};
	image_region_batch regions;
	regions.reset(make_span(element_extents), 3);
	regions.reserve(3);

	const std::vector<std::size_t> targets = {3, 0, 2};
	for (std::size_t slot = 0; slot < targets.size(); ++slot)
	{
		const std::size_t dataset_offset[3] = {targets[slot], 0, 0};
		const std::size_t array_offset[3] = {slot, 0, 0};
		regions.add(make_span(dataset_offset, 3), make_span(array_offset, 3));
	}

	writer.write(const_array_ref(batch), regions);

	// Slot 0 holds 0..14 and goes to element 3, slot 1 holds 15..29 and goes
	// to element 0, slot 2 holds 30..44 and goes to element 2.
	REQUIRE( writer.get_element(45) == 0 );
	REQUIRE( writer.get_element(0) == 15 );
	REQUIRE( writer.get_element(30) == 30 );
	REQUIRE( writer.get_element(15) == 0 );
}

TEST_CASE( "a write converts from the source data type", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));
	const std::vector<std::size_t> extents = {1, 1, 4};
	const std::size_t origin[3] = {0, 0, 0};

	SECTION( "a float source is converted preserving the value" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::float32,
			7.0f
		);
		writer.write(
			const_array_ref(source),
			make_regions(
				make_span(extents),
				make_span(origin, 3),
				make_span(origin, 3)
			)
		);

		REQUIRE( writer.get_element(0) == 7 );
		REQUIRE( writer.get_element(3) == 10 );
	}

	SECTION( "a data type that can not be written is refused" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::float64,
			1.0f
		);

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source),
				make_regions(
					make_span(extents),
					make_span(origin, 3),
					make_span(origin, 3)
				)
			),
			invalid_operation_error
		);
	}
}

TEST_CASE( "a write reads through a strided source", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	// The batch is wider than the elements taken out of it.
	const std::vector<std::size_t> batch_extents = {2, 3, 10};
	auto batch = make_source(
		make_span(batch_extents),
		numerical_type::int16,
		0.0f
	);

	const std::vector<std::size_t> element_extents = {1, 3, 5};
	image_region_batch regions;
	regions.reset(make_span(element_extents), 3);
	const std::size_t dataset_offset[3] = {0, 0, 0};
	const std::size_t array_offset[3] = {0, 0, 5};
	regions.add(make_span(dataset_offset, 3), make_span(array_offset, 3));

	writer.write(const_array_ref(batch), regions);

	// Column 5 of row 0 of the batch is value 5, and the next row starts at
	// 15, so the strided source is followed rather than a flat run copied.
	REQUIRE( writer.get_element(0) == 5 );
	REQUIRE( writer.get_element(4) == 9 );
	REQUIRE( writer.get_element(5) == 15 );
}

TEST_CASE( "a write refuses a region it can not place", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));
	const std::vector<std::size_t> extents = {1, 3, 5};
	const std::size_t origin[3] = {0, 0, 0};

	SECTION( "a region reaching past the dataset is out of range" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			0.0f
		);
		const std::size_t dataset_offset[3] = {4, 0, 0};

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source),
				make_regions(
					make_span(extents),
					make_span(dataset_offset, 3),
					make_span(origin, 3)
				)
			),
			std::out_of_range
		);
	}

	SECTION( "a region reaching past the source is out of range" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			0.0f
		);
		const std::size_t array_offset[3] = {1, 0, 0};

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source),
				make_regions(
					make_span(extents),
					make_span(origin, 3),
					make_span(array_offset, 3)
				)
			),
			std::out_of_range
		);
	}

	SECTION( "extents of the wrong rank are refused, never adjusted" )
	{
		const std::vector<std::size_t> flat_extents = {3, 5};
		auto source = make_source(
			make_span(flat_extents),
			numerical_type::int16,
			0.0f
		);

		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(source),
				make_regions(
					make_span(extents),
					make_span(origin, 3),
					make_span(origin, 3)
				)
			),
			std::invalid_argument
		);
	}

	SECTION( "an uninitialized source is refused" )
	{
		REQUIRE_THROWS_AS(
			writer.write(
				const_array_ref(),
				make_regions(
					make_span(extents),
					make_span(origin, 3),
					make_span(origin, 3)
				)
			),
			std::invalid_argument
		);
	}
}

TEST_CASE( "a writer is opened over a complete descriptor", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	SECTION( "the descriptor bounds what may be written" )
	{
		std::vector<std::size_t> extents;
		writer.get_descriptor().get_layout().get_extents(extents);

		REQUIRE( extents == dataset_extents );
	}

	SECTION( "flushing is explicit so that a failure can be reported" )
	{
		REQUIRE( writer.get_flush_count() == 0 );
		writer.flush();
		REQUIRE( writer.get_flush_count() == 1 );
	}
}

TEST_CASE( "image_writer is mockable", "[image_writer]" )
{
	mock_image_writer writer;

	REQUIRE_CALL(writer, write(ANY(const_array_ref),
		ANY(const image_region_batch&)));
	REQUIRE_CALL(writer, flush());

	image_writer &interface = writer;
	const image_region_batch regions;
	interface.write(const_array_ref(), regions);
	interface.flush();
}
