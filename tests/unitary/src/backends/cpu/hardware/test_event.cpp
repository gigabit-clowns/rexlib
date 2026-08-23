// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/hardware/event.hpp>

using namespace rex;
using namespace rex::cpu;

TEST_CASE( "event should support host_query, host_wait and device_wait", "[cpu::event]" )
{
	cpu::event evt;
	const auto usage = evt.get_supported_usage();

	REQUIRE( usage.contains(event_usage_flag_bits::host_query) );
	REQUIRE( usage.contains(event_usage_flag_bits::host_wait) );
	REQUIRE( usage.contains(event_usage_flag_bits::device_wait) );
}

TEST_CASE( "event should not advertise cross_device_wait", "[cpu::event]" )
{
	cpu::event evt;
	REQUIRE_FALSE(
		evt.get_supported_usage().contains(event_usage_flag_bits::cross_device_wait)
	);
}

TEST_CASE( "cpu::event::wait should return immediately", "[cpu::event]" )
{
	cpu::event evt;
	REQUIRE_NOTHROW( evt.wait() );
}

TEST_CASE( "cpu::event::is_signaled should always return true", "[cpu::event]" )
{
	cpu::event evt;
	REQUIRE( evt.is_signaled() );
}
