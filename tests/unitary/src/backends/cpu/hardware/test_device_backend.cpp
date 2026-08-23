// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/hardware/device_backend.hpp>

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/core_version.hpp>

#include <stdexcept>

using namespace rex;
using namespace rex::cpu;

TEST_CASE(
	"cpu::device_backend should be named as cpu",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;
	REQUIRE( backend.get_name() == "cpu" );
}

TEST_CASE(
	"cpu::device_backend should have the same version as the core",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;
	REQUIRE( backend.get_version() == xmipp4::get_core_version() );
}

TEST_CASE(
	"cpu::device_backend should have a single device id",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;

	std::vector<std::size_t> device_ids;
	backend.enumerate_devices(device_ids);

	REQUIRE( device_ids.size() == 1 );
	REQUIRE( device_ids[0] == 0 );
}

TEST_CASE(
	"cpu::device_backend should fail gathering info about a device id "
	"unequal to 0",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;

	const std::size_t id = GENERATE(1, 2, 10, 32);
	device_properties properties;

	REQUIRE( backend.get_device_properties(id, properties) == false );
}

TEST_CASE(
	"cpu::device_backend should gather info about a device with an id "
	"equal to 0",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;

	device_properties properties;
	REQUIRE( backend.get_device_properties(0, properties) == true );
}

TEST_CASE(
	"cpu::device_backend should throw creating a device with an id "
	"unequal to 0",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;

	const std::size_t id = GENERATE(1, 2, 10, 32);
	device_properties properties;

	REQUIRE_THROWS_MATCHES(
		backend.create_device(id),
		std::invalid_argument,
		Catch::Matchers::Message(
			"Requested device id is invalid"
		)
	);
}

TEST_CASE(
	"cpu::device_backend should create a valid device with an id equal to 0",
	"[cpu::device_backend]"
)
{
	cpu::device_backend backend;

	const auto device = backend.create_device(0);
	REQUIRE( device != nullptr );
}
