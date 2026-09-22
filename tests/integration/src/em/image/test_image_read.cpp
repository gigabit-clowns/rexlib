// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/em/image/direct_image_reader_provider.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/index_table.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/tests/assets.hpp>

#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// EMD-3197 is a volume of twenty cubed, so a patch of it is a subtomogram
// and the whole file fits in memory twice over.
const std::size_t volume_extent = 20;
const std::size_t box_extent = 6;

std::size_t volume_index(std::size_t z, std::size_t y, std::size_t x)
{
	return (z * volume_extent + y) * volume_extent + x;
}

std::size_t box_index(
	std::size_t slot,
	std::size_t z,
	std::size_t y,
	std::size_t x
)
{
	return ((slot * box_extent + z) * box_extent + y) * box_extent + x;
}

void add_position(
	index_table &positions,
	std::size_t z,
	std::size_t y,
	std::size_t x
)
{
	const std::size_t values[3] = {z, y, x};
	positions.add(make_span(values, 3));
}

} // anonymous namespace

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"patches cut from a real MRC file hold what the file holds",
	"[mrc][image_read]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();
	const auto path = get_mrc_asset_path("EMD-3197.map");
	const image_location location(path);

	const auto whole = em::read(location, *manager, context);
	REQUIRE( whole.get_descriptor().get_layout().get_rank() == 3 );
	const auto volume = read_host<float>(
		whole,
		volume_extent * volume_extent * volume_extent
	);

	const auto readers =
		std::make_shared<direct_image_reader_provider>(manager);
	const auto source = std::make_shared<image_source>(
		readers,
		std::make_shared<synchronous_executor>()
	);

	// One box well inside the volume and one centred so near a corner that
	// it begins two samples outside it along every axis.
	index_table positions(3);
	add_position(positions, 10, 10, 10);
	add_position(positions, 1, 1, 1);

	const std::vector<std::size_t> batch_extents = {
		2, box_extent, box_extent, box_extent
	};
	auto destination = full(
		make_descriptor(batch_extents, numerical_type::float32),
		memory_resource_affinity::host,
		std::numeric_limits<float>::quiet_NaN(),
		context
	);

	const auto completion =
		read_patches_async(
			*source,
			destination.share(),
			location,
			positions
		);
	REQUIRE( completion != nullptr );
	REQUIRE_NOTHROW( completion->get() );

	const auto boxes = read_host<float>(
		destination,
		2 * box_extent * box_extent * box_extent
	);

	SECTION( "a box inside the volume holds the samples around its centre" )
	{
		// Centred at ten with an extent of six, so it begins at seven.
		for (std::size_t z = 0; z < box_extent; ++z)
		{
			for (std::size_t y = 0; y < box_extent; ++y)
			{
				for (std::size_t x = 0; x < box_extent; ++x)
				{
					REQUIRE(
						boxes[box_index(0, z, y, x)] ==
						volume[volume_index(7 + z, 7 + y, 7 + x)]
					);
				}
			}
		}
	}

	SECTION( "a box over a corner holds the part of it the volume covers" )
	{
		// Centred at one with an extent of six, so it begins at minus two
		// and the volume reaches only the last four samples of each axis.
		for (std::size_t z = 0; z < box_extent; ++z)
		{
			for (std::size_t y = 0; y < box_extent; ++y)
			{
				for (std::size_t x = 0; x < box_extent; ++x)
				{
					const auto covered = z >= 2 && y >= 2 && x >= 2;
					const auto value = boxes[box_index(1, z, y, x)];

					if (covered)
					{
						REQUIRE(
							value ==
							volume[volume_index(z - 2, y - 2, x - 2)]
						);
					}
					else
					{
						REQUIRE( std::isnan(value) );
					}
				}
			}
		}
	}
}
