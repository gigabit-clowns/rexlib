// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/hardware/device_properties.hpp>

#include <array>

using namespace rexlib;

TEST_CASE(
	"setting and getting attributes in device_properties should preserve them",
	"[hardware]"
)
{
	device_properties prop;
	prop.set_name("REXLIB A1234 SuperFast");
	prop.set_type(device_type::gpu);
	prop.set_physical_location("My computer");
	prop.set_total_memory_bytes(0xDEADBEEF);
	prop.set_optimal_data_alignment(2048);

	CHECK( prop.get_name() == "REXLIB A1234 SuperFast" );
	CHECK( prop.get_type() == device_type::gpu );
	CHECK( prop.get_physical_location() == "My computer" );
	CHECK( prop.get_total_memory_bytes() == 0xDEADBEEF );
	CHECK( prop.get_optimal_data_alignment() == 2048 );
}

TEST_CASE(
	"assigning device_properties without attributes should clear the "
	"attributes of the target",
	"[hardware]"
)
{
	device_properties prop;
	prop.set_name("REXLIB A1234 SuperFast");
	const device_properties empty;

	prop = empty;

	CHECK( prop.get_name().empty() );
}

TEST_CASE(
	"assigning device_properties to themselves should keep their attributes",
	"[hardware]"
)
{
	device_properties prop;
	prop.set_name("REXLIB A1234 SuperFast");
	const auto &self = prop;

	prop = self;

	CHECK( prop.get_name() == "REXLIB A1234 SuperFast" );
}
