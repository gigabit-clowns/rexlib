// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"
#include "fixtures/scoped_path.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/tests/assets.hpp>

#include <cstdio>
#include <cstring>
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
		make_span(extents), extents.size(), extents.size());
	regions.add(
		make_span(std::vector<std::size_t>(extents.size(), 0)),
		make_span(std::vector<std::size_t>(extents.size(), 0))
	);

	return regions;
}

} // anonymous namespace

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"an MRC file created through the managers reads back as it was written",
	"[mrc][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();

	// The four shapes an MRC file can hold. The extents of the stack and of
	// the volume are the same, so what tells them apart is the core rank
	// alone, which is the distinction the whole round trip is here to check.
	struct shape
	{
		std::vector<std::size_t> extents;
		std::size_t core_rank;
	};

	const std::vector<shape> shapes = {
		{{3, 4}, 2},
		{{2, 3, 4}, 2},
		{{2, 3, 4}, 3},
		{{2, 3, 4, 5}, 3}
	};

	for (const auto &subject : shapes)
	{
		const scoped_path path("round_trip_managers.mrc");
		const auto values = counting(element_count(subject.extents));

		auto source = zeros(
			make_descriptor(subject.extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		std::memcpy(
			source.get_storage()->get_host_ptr(),
			values.data(),
			values.size() * sizeof(float)
		);

		const image_descriptor descriptor(
			make_span(subject.extents),
			subject.core_rank,
			numerical_type::float32
		);

		{
			const auto writer =
				writers->open(path.get(), descriptor, image_metadata());
			writer->write(
				const_array_ref(source), whole_of(subject.extents));
			writer->flush();
		}

		const auto reader = readers->open(path.get());

		REQUIRE( reader->get_descriptor() == descriptor );

		auto destination = zeros(
			make_descriptor(subject.extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(subject.extents));

		REQUIRE( read_host<float>(destination, values.size()) == values );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"an MRC file converts to and from the type it holds",
	"[mrc][image_format_manager]" )
{
	const auto writers =
		catalog.get_service_manager<image_write_format_manager>();
	const auto readers =
		catalog.get_service_manager<image_read_format_manager>();

	const std::vector<std::size_t> extents = {2, 2};
	const std::vector<float> values = {-2.0F, -1.0F, 0.0F, 300.0F};

	const numerical_type file_types[] = {
		numerical_type::int16,
		numerical_type::float32
	};

	for (const auto file_type : file_types)
	{
		const scoped_path path("round_trip_conversion.mrc");

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

		const auto read = read_host<double>(destination, values.size());

		REQUIRE( read == std::vector<double>(
			values.begin(), values.end()) );
	}
}
