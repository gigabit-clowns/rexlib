// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/hardware/device_type.hpp>

#include <string>

using namespace rex;

TEST_CASE("converting device_type to string should produce the expected string")
{
	REQUIRE( to_string(device_type::unknown) == std::string("unknown") );
	REQUIRE( to_string(device_type::cpu) == std::string("CPU") );
	REQUIRE( to_string(device_type::gpu) == std::string("GPU") );
	REQUIRE( to_string(device_type::integrated_gpu) == std::string("iGPU") );
}
