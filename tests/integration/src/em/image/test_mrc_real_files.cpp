// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/tests/assets.hpp>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<char> read_bytes(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	return std::vector<char>(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>()
	);
}

// A second reading of the header, written against the specification rather
// than against the parser, so that a file is compared with what it says it
// holds and not with what the parser decided it holds. Both assets are
// little-endian, as is every machine the suite runs on.
std::int32_t field(const std::vector<char> &raw, std::size_t offset)
{
	std::int32_t value = 0;
	std::memcpy(&value, raw.data() + offset, sizeof(value));
	return value;
}

std::vector<float> values_of(const std::string &path)
{
	const auto raw = read_bytes(path);
	const auto offset = 1024 + static_cast<std::size_t>(field(raw, 92));
	const auto count = (raw.size() - offset) / sizeof(float);

	std::vector<float> values(count);
	std::memcpy(values.data(), raw.data() + offset, count * sizeof(float));
	return values;
}

// The values of a volume along the three axes of space, the one along X
// changing fastest. The columns of the file change fastest and run along the
// axis MAPC names, its rows along MAPR and its sections along MAPS, so the
// three are walked in whichever order puts the axes of space in that one.
std::vector<float> values_along_space_of(const std::string &path)
{
	const auto raw = read_bytes(path);
	const auto values = values_of(path);

	const std::size_t columns = static_cast<std::size_t>(field(raw, 0));
	const std::size_t rows = static_cast<std::size_t>(field(raw, 4));
	const std::size_t sections = static_cast<std::size_t>(field(raw, 8));
	const std::size_t counts[3] = {columns, rows, sections};
	const std::size_t stored_strides[3] = {1, columns, columns * rows};

	std::size_t extents[3] = {0, 0, 0};
	std::size_t strides[3] = {0, 0, 0};
	for (std::size_t stored = 0; stored < 3; ++stored)
	{
		const auto axis =
			static_cast<std::size_t>(field(raw, 64 + 4 * stored)) - 1;
		extents[axis] = counts[stored];
		strides[axis] = stored_strides[stored];
	}

	std::vector<float> ordered;
	ordered.reserve(values.size());
	for (std::size_t z = 0; z < extents[2]; ++z)
	{
		for (std::size_t y = 0; y < extents[1]; ++y)
		{
			for (std::size_t x = 0; x < extents[0]; ++x)
			{
				ordered.push_back(
					values[z * strides[2] + y * strides[1] + x * strides[0]]);
			}
		}
	}

	return ordered;
}

std::size_t element_count(span<const std::size_t> extents)
{
	return std::accumulate(
		extents.begin(),
		extents.end(),
		std::size_t(1),
		std::multiplies<std::size_t>()
	);
}

} // anonymous namespace

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"MRC files this project did not write are read as they state",
	"[mrc][image_format_manager]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();

	SECTION( "a pre-2014 volume is claimed and read" )
	{
		// EMD-3197 carries nversion = 0 and the machine stamp 0x44 0x41,
		// neither of which MRC2014 specifies. Both are accepted.
		const auto path = get_mrc_asset_path("EMD-3197.map");
		const auto raw = read_bytes(path);

		REQUIRE( field(raw, 88) == 1 );
		REQUIRE( field(raw, 92) == 0 );

		REQUIRE( manager->get_most_suitable_format(image_probe(path)) !=
			nullptr );

		const auto reader = manager->open(path);

		const std::vector<std::size_t> extents = {20, 20, 20};

		REQUIRE( reader->get_descriptor() ==
			image_descriptor(make_span(extents), 3, numerical_type::float32) );
	}

	SECTION( "a crystallographic volume does not read its sampling as depth" )
	{
		// EMD-3001 is space group 4, and its grid sampling (40, 12, 72) has
		// nothing to do with its extents. Reading MZ as a count of sections
		// per volume would give this file a fourth axis it does not have.
		const auto path = get_mrc_asset_path("EMD-3001.map");
		const auto raw = read_bytes(path);

		REQUIRE( field(raw, 88) == 4 );
		REQUIRE( field(raw, 36) == 72 );
		REQUIRE( field(raw, 92) == 160 );

		const auto reader = manager->open(path);

		REQUIRE( reader->get_descriptor().get_core_rank() == 3 );
		REQUIRE( reader->get_descriptor().get_extents().size() == 3 );
	}

	SECTION( "a volume is read along the axes its header names" )
	{
		// The columns of EMD-3001 run along Z, its rows along X and its
		// sections along Y, so its counts of 73, 43 and 25 are extents of 43
		// along X, 25 along Y and 73 along Z.
		const auto path = get_mrc_asset_path("EMD-3001.map");
		const auto raw = read_bytes(path);

		REQUIRE( field(raw, 64) == 3 );
		REQUIRE( field(raw, 68) == 1 );
		REQUIRE( field(raw, 72) == 2 );

		const auto reader = manager->open(path);
		const auto extents = reader->get_descriptor().get_extents();

		REQUIRE( extents.size() == 3 );
		REQUIRE( extents[0] == 73 );
		REQUIRE( extents[1] == 25 );
		REQUIRE( extents[2] == 43 );
	}
}

TEST_CASE_METHOD( cpu_execution_context_fixture,
	"the values of a real MRC file arrive along the axes its header names",
	"[mrc][image_format_manager]" )
{
	const auto manager =
		catalog.get_service_manager<image_read_format_manager>();

	const std::string names[] = {"EMD-3197.map", "EMD-3001.map"};
	for (const auto &name : names)
	{
		const auto path = get_mrc_asset_path(name);
		const auto reader = manager->open(path);
		const auto extents = reader->get_descriptor().get_extents();
		const std::vector<std::size_t> shape(
			extents.begin(), extents.end());

		auto destination = zeros(
			make_descriptor(shape, numerical_type::float32),
			memory_resource_affinity::host,
			context
		);

		image_transfer_plan regions(
			make_span(shape), shape.size(), shape.size());
		regions.add(
			make_span(std::vector<std::size_t>(shape.size(), 0)),
			make_span(std::vector<std::size_t>(shape.size(), 0))
		);

		reader->read(array_ref(destination), regions);

		const auto count = element_count(extents);
		const auto expected = values_along_space_of(path);

		REQUIRE( expected.size() == count );
		REQUIRE( read_host<float>(destination, count) == expected );
	}
}
