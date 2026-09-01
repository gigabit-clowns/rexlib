// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_writer.hpp>

#include "fake/fake_image_writer.hpp"
#include "mock/mock_image_writer.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
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

} // namespace

TEST_CASE( "a write places one region of the dataset", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	SECTION( "an element of a stack lands where it belongs" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			100.0f
		);
		const std::vector<std::size_t> offset = {2, 0, 0};
		writer.write_region(make_span(offset), const_array_ref(source));

		REQUIRE( writer.get_element(30) == 100 );
		REQUIRE( writer.get_element(44) == 114 );
		REQUIRE( writer.get_element(29) == 0 );
	}

	SECTION( "a region that is never written keeps the laid out value" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			1.0f
		);
		const std::vector<std::size_t> offset = {0, 0, 0};
		writer.write_region(make_span(offset), const_array_ref(source));

		REQUIRE( writer.get_element(15) == 0 );
		REQUIRE( writer.get_element(59) == 0 );
	}

	SECTION( "regions may be written in any order" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		const std::vector<std::size_t> order = {3, 0, 2};
		for (const auto plane : order)
		{
			auto source = make_source(
				make_span(extents),
				numerical_type::int16,
				static_cast<float>(plane * 100)
			);
			const std::vector<std::size_t> offset = {plane, 0, 0};
			writer.write_region(make_span(offset), const_array_ref(source));
		}

		REQUIRE( writer.get_element(0) == 0 );
		REQUIRE( writer.get_element(30) == 200 );
		REQUIRE( writer.get_element(45) == 300 );
	}
}

TEST_CASE( "a write converts from the source data type", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));
	const std::vector<std::size_t> extents = {1, 1, 4};
	const std::vector<std::size_t> offset = {0, 0, 0};

	SECTION( "a float source is converted preserving the value" )
	{
		auto source = make_source(
			make_span(extents),
			numerical_type::float32,
			7.0f
		);
		writer.write_region(make_span(offset), const_array_ref(source));

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
			writer.write_region(make_span(offset), const_array_ref(source)),
			invalid_operation_error
		);
	}
}

TEST_CASE( "a write reads through a strided source", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	const std::vector<std::size_t> batch_extents = {2, 1, 3, 5};
	auto batch = make_source(
		make_span(batch_extents),
		numerical_type::int16,
		0.0f
	);

	for (std::size_t slot = 0; slot < 2; ++slot)
	{
		const std::vector<dynamic_subscript> subscripts = {
			static_cast<std::ptrdiff_t>(slot), ellipsis()
		};
		const auto view = subarray(
			const_array_ref(batch),
			make_span(subscripts)
		);

		const std::vector<std::size_t> offset = {slot, 0, 0};
		writer.write_region(make_span(offset), const_array_ref(view));
	}

	REQUIRE( writer.get_element(0) == 0 );
	REQUIRE( writer.get_element(14) == 14 );
	REQUIRE( writer.get_element(15) == 15 );
	REQUIRE( writer.get_element(29) == 29 );
}

TEST_CASE( "a write refuses a region it can not place", "[image_writer]" )
{
	fake_image_writer writer(make_span(dataset_extents));

	SECTION( "a region reaching past the dataset is out of range" )
	{
		const std::vector<std::size_t> extents = {1, 3, 5};
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			0.0f
		);
		const std::vector<std::size_t> offset = {4, 0, 0};

		REQUIRE_THROWS_AS(
			writer.write_region(make_span(offset), const_array_ref(source)),
			std::out_of_range
		);
	}

	SECTION( "a source of the wrong rank is refused, never adjusted" )
	{
		const std::vector<std::size_t> extents = {3, 5};
		auto source = make_source(
			make_span(extents),
			numerical_type::int16,
			0.0f
		);
		const std::vector<std::size_t> offset = {0, 0, 0};

		REQUIRE_THROWS_AS(
			writer.write_region(make_span(offset), const_array_ref(source)),
			std::invalid_argument
		);
	}

	SECTION( "an uninitialized source is refused" )
	{
		const std::vector<std::size_t> offset = {0, 0, 0};

		REQUIRE_THROWS_AS(
			writer.write_region(make_span(offset), const_array_ref()),
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

	REQUIRE_CALL(writer, write_region(ANY(span<const std::size_t>),
		ANY(const_array_ref)));
	REQUIRE_CALL(writer, flush());

	image_writer &interface = writer;
	const std::vector<std::size_t> offset = {0, 0, 0};
	interface.write_region(make_span(offset), const_array_ref());
	interface.flush();
}
