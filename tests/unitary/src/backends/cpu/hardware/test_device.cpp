// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/backends/cpu/device.hpp>

#include <backends/cpu/hardware/command_queue.hpp>
#include <backends/cpu/hardware/event.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/event.hpp>

#include "../serial_pool.hpp"

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"cpu::device should expose a single memory resource",
	"[cpu::device]"
)
{
	cpu::device dev;

	const auto affinity = GENERATE(
		memory_resource_affinity::host,
		memory_resource_affinity::device
	);
	REQUIRE(
		&dev.get_memory_resource(affinity) == &get_host_memory_resource()
	);
}

TEST_CASE(
	"cpu::device should create a non-null command_queue",
	"[cpu::device]"
)
{
	cpu::device dev;
	const auto queue = dev.create_command_queue();

	REQUIRE( queue != nullptr );
	REQUIRE( dynamic_cast<cpu::command_queue*>(queue.get()) != nullptr );
}

TEST_CASE(
	"cpu::device should create an independent command_queue on each call",
	"[cpu::device]"
)
{
	cpu::device dev;
	const auto queue_a = dev.create_command_queue();
	const auto queue_b = dev.create_command_queue();

	REQUIRE( queue_a != nullptr );
	REQUIRE( queue_b != nullptr );
	CHECK( queue_a != queue_b );
}

TEST_CASE(
	"cpu::device should run every queue it creates over its own pool",
	"[cpu::device]"
)
{
	const cpu::device dev(get_serial_pool());
	const auto queue = dev.create_command_queue();

	const auto *cpu_queue =
		cpu::command_queue::try_cast(*queue);
	REQUIRE( cpu_queue != nullptr );
	CHECK( &cpu_queue->get_thread_pool() == get_serial_pool().get() );
}

TEST_CASE(
	"cpu::device should build its own pool when none is named",
	"[cpu::device]"
)
{
	// Nothing static holds this one: it is spawned here and joined when the
	// device below goes out of scope, which is the whole point of the device
	// owning it.
	const cpu::device dev;

	REQUIRE( dev.get_thread_pool() != nullptr );
	CHECK( dev.get_thread_pool()->get_size() >= 1 );
}

TEST_CASE(
	"cpu::device should create a non-null event",
	"[cpu::device]"
)
{
	cpu::device dev;
	const auto evt = dev.create_event({});

	REQUIRE( evt != nullptr );
	REQUIRE( dynamic_cast<cpu::event*>(evt.get()) != nullptr );
}

TEST_CASE(
	"cpu::device should ignore the requested event usage flags",
	"[cpu::device]"
)
{
	cpu::device dev;
	const auto evt = dev.create_event({
		event_usage_flag_bits::host_query,
		event_usage_flag_bits::host_wait,
		event_usage_flag_bits::device_wait
	});

	REQUIRE( evt != nullptr );
	const auto usage = evt->get_supported_usage();
	REQUIRE( usage.contains(event_usage_flag_bits::host_query) );
	REQUIRE( usage.contains(event_usage_flag_bits::host_wait) );
	REQUIRE( usage.contains(event_usage_flag_bits::device_wait) );
}
