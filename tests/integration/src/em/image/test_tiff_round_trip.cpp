// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"
#include "fixtures/scoped_path.hpp"

#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/direct_image_reader_provider.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_read.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/em/image/image_write.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/tests/assets.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

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

std::vector<float> counting(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<float>(i) + 0.25F;
	}

	return values;
}

image_transfer_plan whole_of(const std::vector<std::size_t> &extents)
{
	image_transfer_plan regions(
		image_transfer_shape(extents, extents.size(), extents.size())
	);
	regions.add(
		make_span(std::vector<std::size_t>(extents.size(), 0)),
		make_span(std::vector<std::size_t>(extents.size(), 0))
	);

	return regions;
}

} // anonymous namespace

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"a TIFF file created through the managers reads back as it was written",
	"[tiff][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();

	// The two shapes a TIFF file can hold, under both of its extensions.
	const std::vector<std::vector<std::size_t>> shapes = {
		{3, 4},
		{2, 3, 4}
	};
	const std::vector<std::string> names = {
		"round_trip_managers.tif",
		"round_trip_managers.tiff"
	};

	for (const auto &name : names)
	{
		for (const auto &extents : shapes)
		{
			const scoped_path path(name);
			const auto values = counting(element_count(extents));

			auto source = zeros(
				make_descriptor(extents, numerical_type::float32),
				memory_resource_affinity::host,
				context
			);
			std::memcpy(
				source.get_storage()->get_host_ptr(),
				values.data(),
				values.size() * sizeof(float)
			);

			const image_descriptor descriptor(
				make_span(extents),
				2,
				numerical_type::float32
			);

			{
				const auto writer =
					writers->open(path.get(), descriptor, image_metadata());
				writer->write(const_array_ref(source), whole_of(extents));
				writer->flush();
			}

			const auto reader = readers->open(path.get());

			REQUIRE( reader->get_descriptor() == descriptor );

			auto destination = zeros(
				make_descriptor(extents, numerical_type::float32),
				memory_resource_affinity::host,
				context
			);
			reader->read(array_ref(destination), whole_of(extents));

			REQUIRE( read_host<float>(destination, values.size()) ==
				values );
		}
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"a TIFF file converts to and from the type it holds",
	"[tiff][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();

	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<float> values = {0.0F, 1.0F, 100.0F, 127.0F};

	const numerical_type file_types[] = {
		numerical_type::int8,
		numerical_type::uint8,
		numerical_type::int16,
		numerical_type::uint16,
		numerical_type::float16,
		numerical_type::float32
	};

	for (const auto file_type : file_types)
	{
		const scoped_path path("round_trip_conversion.tif");

		auto source = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		std::memcpy(
			source.get_storage()->get_host_ptr(),
			values.data(),
			values.size() * sizeof(float)
		);

		{
			const auto writer = writers->open(
				path.get(),
				image_descriptor(make_span(extents), 2, file_type),
				image_metadata()
			);
			writer->write(const_array_ref(source), whole_of(extents));
			writer->flush();
		}

		const auto reader = readers->open(path.get());

		REQUIRE( reader->get_descriptor().get_data_type() == file_type );

		// Read back into a wider type than the file holds, which is the
		// conversion a caller asks for rather than the one the file forces.
		auto destination = zeros(
			make_descriptor(extents, numerical_type::float64),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<double>(destination, values.size()) ==
			std::vector<double>(values.begin(), values.end()) );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"a TIFF stack is written a page at a time and read back whole",
	"[tiff][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();

	const scoped_path path("round_trip_paged.tif");
	const std::vector<std::size_t> extents = {4, 3, 5};
	const std::vector<std::size_t> page = {3, 5};
	const auto values = counting(element_count(extents));

	auto source = zeros(
		make_descriptor(extents, numerical_type::float32),
		memory_resource_affinity::host,
		context
	);
	std::memcpy(
		source.get_storage()->get_host_ptr(),
		values.data(),
		values.size() * sizeof(float)
	);

	{
		const auto writer = writers->open(
			path.get(),
			image_descriptor(make_span(extents), 2, numerical_type::int16),
			image_metadata()
		);

		for (std::size_t index = 0; index < extents[0]; ++index)
		{
			image_transfer_plan regions(image_transfer_shape(page, 3, 3));
			regions.add(
				make_span(std::vector<std::size_t>{index, 0, 0}),
				make_span(std::vector<std::size_t>{index, 0, 0})
			);
			writer->write(const_array_ref(source), regions);
		}
	}

	const auto reader = readers->open(path.get());
	auto destination = zeros(
		make_descriptor(extents, numerical_type::int16),
		memory_resource_affinity::host,
		context
	);
	reader->read(array_ref(destination), whole_of(extents));

	// The file holds integers, so the quarter every value carries is gone.
	std::vector<std::int16_t> expected(values.size());
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		expected[i] = static_cast<std::int16_t>(i);
	}

	REQUIRE( read_host<std::int16_t>(destination, values.size()) ==
		expected );
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"a TIFF file is written and read by the whole array functions",
	"[tiff][image_write][image_read]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers = std::make_shared<direct_image_reader_provider>(
		catalog.get_service_manager<image_read_format_manager>());

	SECTION( "a stack" )
	{
		const scoped_path path("round_trip_whole_stack.tif");
		const std::vector<std::size_t> extents = {3, 4, 5};
		const auto values = counting(element_count(extents));

		auto source = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		std::memcpy(
			source.get_storage()->get_host_ptr(),
			values.data(),
			values.size() * sizeof(float)
		);

		write_stack(source, path.get(), *writers);

		const auto read = em::read(path.get(), *readers, context);

		REQUIRE( read.get_descriptor() ==
			make_descriptor(extents, numerical_type::float32) );
		REQUIRE( read_host<float>(read, values.size()) == values );
	}

	SECTION( "a single image" )
	{
		const scoped_path path("round_trip_whole_single.tif");
		const std::vector<std::size_t> extents = {4, 5};
		const auto values = counting(element_count(extents));

		auto source = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		std::memcpy(
			source.get_storage()->get_host_ptr(),
			values.data(),
			values.size() * sizeof(float)
		);

		write_single(source, path.get(), *writers);

		const auto read = em::read(path.get(), *readers, context);

		REQUIRE( read_host<float>(read, values.size()) == values );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"what a TIFF file can not hold is refused through the managers",
	"[tiff][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const scoped_path path("round_trip_refused.tif");

	SECTION( "a stack of one image" )
	{
		const std::vector<std::size_t> extents = {1, 3, 4};

		REQUIRE_THROWS_AS(
			writers->open(
				path.get(),
				image_descriptor(
					make_span(extents), 2, numerical_type::float32),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "a volume" )
	{
		const std::vector<std::size_t> extents = {2, 3, 4};

		REQUIRE_THROWS_AS(
			writers->open(
				path.get(),
				image_descriptor(
					make_span(extents), 3, numerical_type::float32),
				image_metadata()
			),
			unsupported_operation_error
		);
	}

	SECTION( "part of a page" )
	{
		const std::vector<std::size_t> extents = {3, 4};
		const std::vector<std::size_t> half = {3, 2};

		auto source = zeros(
			make_descriptor(half, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		const auto writer = writers->open(
			path.get(),
			image_descriptor(make_span(extents), 2, numerical_type::float32),
			image_metadata()
		);

		image_transfer_plan regions(image_transfer_shape(half, 2, 2));
		regions.add(
			make_span(std::vector<std::size_t>{0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		REQUIRE_THROWS_AS(
			writer->write(const_array_ref(source), regions),
			unsupported_operation_error
		);
	}
}
