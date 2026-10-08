// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/tests/assets.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// The samples of each asset, restated from the formulas its README gives
// rather than read off the file by other means.
std::vector<std::uint8_t> uint8_samples(std::size_t count)
{
	std::vector<std::uint8_t> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<std::uint8_t>((7 * i + 3) % 251);
	}

	return values;
}

std::vector<std::uint16_t> uint16_samples(std::size_t count)
{
	std::vector<std::uint16_t> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<std::uint16_t>(1000 + 257 * i);
	}

	return values;
}

std::vector<std::int16_t> int16_samples(std::size_t count)
{
	std::vector<std::int16_t> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = static_cast<std::int16_t>(37 * static_cast<int>(i) - 300);
	}

	return values;
}

std::vector<float> float32_samples(std::size_t count)
{
	std::vector<float> values(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		values[i] = 0.25F * static_cast<float>(i) - 3.5F;
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
	"TIFF files rexlib did not write are read as they state",
	"[tiff][image_format_manager]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();

	SECTION( "a compressed stack cut into several strips" )
	{
		const auto path = get_tiff_asset_path("stack_uint8_lzw.tif");
		const std::vector<std::size_t> extents = {3, 6, 8};

		REQUIRE( manager->get_most_suitable_format(image_probe(path)) !=
			nullptr );

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 2, numerical_type::uint8) );

		auto destination = zeros(
			make_descriptor(extents, numerical_type::uint8),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<std::uint8_t>(destination, 144) ==
			uint8_samples(144) );
	}

	SECTION( "an image compressed with Deflate after a predictor" )
	{
		const auto path =
			get_tiff_asset_path("image_uint16_deflate_predictor.tif");
		const std::vector<std::size_t> extents = {6, 8};

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 2, numerical_type::uint16) );

		auto destination = zeros(
			make_descriptor(extents, numerical_type::uint16),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<std::uint16_t>(destination, 48) ==
			uint16_samples(48) );
	}

	SECTION( "an image cut into tiles that reach past it" )
	{
		const auto path = get_tiff_asset_path("image_float32_tiled.tif");
		const std::vector<std::size_t> extents = {24, 40};

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 2, numerical_type::float32) );

		auto destination = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<float>(destination, 960) ==
			float32_samples(960) );
	}

	SECTION( "an image in the other byte order" )
	{
		const auto path = get_tiff_asset_path("image_int16_big_endian.tif");
		const std::vector<std::size_t> extents = {6, 8};

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 2, numerical_type::int16) );

		auto destination = zeros(
			make_descriptor(extents, numerical_type::int16),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<std::int16_t>(destination, 48) ==
			int16_samples(48) );
	}

	SECTION( "a BigTIFF stack" )
	{
		const auto path = get_tiff_asset_path("stack_uint8_bigtiff.tif");
		const std::vector<std::size_t> extents = {2, 6, 8};

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 2, numerical_type::uint8) );

		auto destination = zeros(
			make_descriptor(extents, numerical_type::uint8),
			memory_resource_affinity::host,
			context
		);
		reader->read(array_ref(destination), whole_of(extents));

		REQUIRE( read_host<std::uint8_t>(destination, 96) ==
			uint8_samples(96) );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"regions of a real TIFF file arrive where they are placed",
	"[tiff][image_format_manager]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();

	SECTION( "one page of a stack, converted on the way" )
	{
		const auto reader =
			manager->open(get_tiff_asset_path("stack_uint8_lzw.tif"));

		const std::vector<std::size_t> extents = {6, 8};
		auto destination = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		image_transfer_plan regions(image_transfer_shape(extents, 3, 2));
		regions.add(
			make_span(std::vector<std::size_t>{2, 0, 0}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader->read(array_ref(destination), regions);

		const auto samples = uint8_samples(144);

		REQUIRE( read_host<float>(destination, 48) ==
			std::vector<float>(samples.begin() + 96, samples.end()) );
	}

	SECTION( "a patch across the tiles of an image" )
	{
		const auto reader =
			manager->open(get_tiff_asset_path("image_float32_tiled.tif"));

		// Two rows from the fifteenth and three columns from the thirty
		// first, where two rows and two columns of tiles meet.
		const std::vector<std::size_t> extents = {2, 3};
		auto destination = zeros(
			make_descriptor(extents, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);
		image_transfer_plan regions(image_transfer_shape(extents, 2, 2));
		regions.add(
			make_span(std::vector<std::size_t>{15, 31}),
			make_span(std::vector<std::size_t>{0, 0})
		);

		reader->read(array_ref(destination), regions);

		const auto samples = float32_samples(960);

		REQUIRE( read_host<float>(destination, 6) ==
			std::vector<float>{
				samples[15 * 40 + 31],
				samples[15 * 40 + 32],
				samples[15 * 40 + 33],
				samples[16 * 40 + 31],
				samples[16 * 40 + 32],
				samples[16 * 40 + 33]
			} );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"well formed TIFF files rexlib does not support are refused",
	"[tiff][image_format_manager]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();

	const std::vector<std::string> names = {
		"refused_rgb.tif",
		"refused_pages_differ.tif"
	};
	for (const auto &name : names)
	{
		const auto path = get_tiff_asset_path(name);

		REQUIRE( manager->get_most_suitable_format(image_probe(path)) !=
			nullptr );
		REQUIRE_THROWS_AS( manager->open(path), image_format_error );
	}
}
