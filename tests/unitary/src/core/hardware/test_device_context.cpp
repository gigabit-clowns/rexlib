// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/hardware/device_context.hpp>

#include <rex/core/hardware/device_session.hpp>
#include <rex/core/hardware/device.hpp>
#include <rex/core/hardware/device_properties.hpp>
#include <rex/core/hardware/memory_allocator.hpp>

#include "mock/mock_device.hpp"
#include "mock/mock_memory_resource.hpp"
#include "mock/mock_memory_allocator.hpp"
#include "mock/mock_command_queue.hpp"

#include <memory>
#include <stdexcept>

using namespace rex;

namespace
{

class device_context_fixture
{
public:
	device_context_fixture()
		: device(std::make_shared<mock_device>())
		, host_allocator(std::make_shared<mock_memory_allocator>())
		, device_allocator(std::make_shared<mock_memory_allocator>())
		, default_queue(std::make_shared<mock_command_queue>())
	{
		REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::host)
		)
			.LR_RETURN(host_resource);
		REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::device)
		)
			.LR_RETURN(device_resource);
		REQUIRE_CALL(host_resource, create_allocator())
			.RETURN(host_allocator);
		REQUIRE_CALL(device_resource, create_allocator())
			.RETURN(device_allocator);
		REQUIRE_CALL(*device, create_command_queue())
			.RETURN(default_queue);

		session = std::make_shared<device_session>(
			device,
			device_properties()
		);
	}

	device_context build_context() const
	{
		return device_context(session);
	}

protected:
	std::shared_ptr<mock_device> device;
	std::shared_ptr<memory_allocator> host_allocator;
	std::shared_ptr<memory_allocator> device_allocator;
	mock_memory_resource host_resource;
	mock_memory_resource device_resource;
	std::shared_ptr<command_queue> default_queue;
	std::shared_ptr<const device_session> session;
};

} // namespace


TEST_CASE(
	"device_context constructor should throw when given a null session",
	"[device_context]"
)
{
	CHECK_THROWS_AS(
		device_context(std::shared_ptr<const device_session>()),
		std::invalid_argument
	);
}

TEST_CASE(
	"device_context default constructor should produce an empty context whose "
	"accessors return null",
	"[device_context]"
)
{
	const device_context context;

	CHECK( context.get_device_session() == nullptr );
	CHECK( context.get_active_queue() == nullptr );
	CHECK( context.get_allocator(memory_resource_affinity::host) == nullptr );
	CHECK( context.get_allocator(memory_resource_affinity::device) == nullptr );
}

TEST_CASE(
	"device_context functional updates on an empty context do not dereference "
	"a missing session",
	"[device_context]"
)
{
	const device_context empty;

	const auto queue = std::make_shared<mock_command_queue>();
	const auto with_queue = empty.on_queue(queue);
	CHECK( with_queue.get_active_queue() == queue );
	CHECK( with_queue.get_device_session() == nullptr );

	// No session to fall back on, so the queue stays null.
	const auto cleared_queue = empty.on_queue(nullptr);
	CHECK( cleared_queue.get_active_queue() == nullptr );

	const auto allocator = std::make_shared<mock_memory_allocator>();
	const auto with_alloc =
		empty.with_allocator(memory_resource_affinity::host, allocator);
	CHECK(
		with_alloc.get_allocator(memory_resource_affinity::host) == allocator
	);
	CHECK( with_alloc.get_device_session() == nullptr );

	// No session to revert to, so the slot stays null.
	const auto cleared_alloc =
		empty.with_allocator(memory_resource_affinity::host, nullptr);
	CHECK(
		cleared_alloc.get_allocator(memory_resource_affinity::host) == nullptr
	);
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context constructor should adopt the default queue and seed an "
	"allocator for each affinity",
	"[device_context]"
)
{
	const auto context = build_context();

	CHECK( context.get_device_session() == session );
	CHECK( context.get_active_queue() == default_queue );
	CHECK(
		context.get_allocator(memory_resource_affinity::host) == host_allocator
	);
	CHECK(
		context.get_allocator(memory_resource_affinity::device)
		== device_allocator
	);
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context on_queue should replace the active queue and preserve the "
	"rest without mutating the receiver",
	"[device_context]"
)
{
	const auto base = build_context();

	const auto other_queue = std::make_shared<mock_command_queue>();
	const auto derived = base.on_queue(other_queue);

	CHECK( derived.get_active_queue() == other_queue );
	CHECK( derived.get_device_session() == session );
	CHECK(
		derived.get_allocator(memory_resource_affinity::host) == host_allocator
	);
	CHECK(
		derived.get_allocator(memory_resource_affinity::device)
		== device_allocator
	);

	// The original context still uses the default queue.
	CHECK( base.get_active_queue() == default_queue );
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context on_queue with null should revert to the device default "
	"queue",
	"[device_context]"
)
{
	const auto base = build_context();

	const auto other_queue = std::make_shared<mock_command_queue>();
	const auto on_custom = base.on_queue(other_queue);
	REQUIRE( on_custom.get_active_queue() == other_queue );

	const auto reverted = on_custom.on_queue(nullptr);

	CHECK( reverted.get_active_queue() == default_queue );
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context with_allocator should override one slot and preserve the "
	"rest without mutating the receiver",
	"[device_context]"
)
{
	const auto base = build_context();

	const auto replacement = std::make_shared<mock_memory_allocator>();
	const auto derived =
		base.with_allocator(memory_resource_affinity::host, replacement);

	CHECK(
		derived.get_allocator(memory_resource_affinity::host) == replacement
	);
	CHECK(
		derived.get_allocator(memory_resource_affinity::device)
		== device_allocator
	);
	CHECK( derived.get_device_session() == session );
	CHECK( derived.get_active_queue() == default_queue );

	// The original context keeps the session allocator.
	CHECK(
		base.get_allocator(memory_resource_affinity::host) == host_allocator
	);
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context with_allocator with null should revert the slot to the "
	"session allocator",
	"[device_context]"
)
{
	const auto base = build_context();

	const auto replacement = std::make_shared<mock_memory_allocator>();
	const auto overridden =
		base.with_allocator(memory_resource_affinity::host, replacement);
	const auto reverted =
		overridden.with_allocator(memory_resource_affinity::host, nullptr);

	CHECK(
		reverted.get_allocator(memory_resource_affinity::host) == host_allocator
	);
	CHECK(
		reverted.get_allocator(memory_resource_affinity::device)
		== device_allocator
	);
}

TEST_CASE_METHOD(
	device_context_fixture,
	"device_context copies share the underlying session, queue and allocators",
	"[device_context]"
)
{
	const auto base = build_context();
	const auto copy = base;

	CHECK( copy.get_device_session() == base.get_device_session() );
	CHECK( copy.get_active_queue() == base.get_active_queue() );
	CHECK(
		copy.get_allocator(memory_resource_affinity::host)
		== base.get_allocator(memory_resource_affinity::host)
	);
	CHECK(
		copy.get_allocator(memory_resource_affinity::device)
		== base.get_allocator(memory_resource_affinity::device)
	);
}
