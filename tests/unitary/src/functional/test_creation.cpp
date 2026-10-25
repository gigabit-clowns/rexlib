// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/creation.hpp>

#include <rexlib/core/ndarray/access_hazard_tracker.hpp>
#include <rexlib/ops/assignment/copy_operation.hpp>
#include <rexlib/ops/assignment/fill_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"
#include "../core/hardware/mock/mock_command_timeline.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using trompeloeil::_;
using rexlib::test::verb_dispatch_fixture;

namespace
{

/**
 * @brief Whether @p op is a fill_operation carrying @p value.
 */
template <typename T>
bool fills_with(const operation &op, T value)
{
	const auto *fill = dynamic_cast<const fill_operation*>(&op);
	return fill && fill->get_fill_value().get<T>() == value;
}

} // anonymous namespace

TEST_CASE(
	"empty throws when the execution context has no allocator for the "
	"requested affinity",
	"[array_creation]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);

	// A default-constructed context is empty: it has no allocators.
	const execution_context context;

	CHECK_THROWS_AS(
		empty(
			descriptor,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty allocates storage from the affinity allocator and wraps it in an "
	"array carrying the requested descriptor",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor() == descriptor );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty makes a new array wait for the commands that still use its "
	"memory",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();
	const auto timeline = std::make_shared<mock_command_timeline>();

	// The allocator hands out memory that an earlier command still uses.
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.LR_SIDE_EFFECT(_4 = command_token(timeline, 5))
		.RETURN(buffer);

	std::vector<command_token> tokens;

	SECTION("when the array is returned")
	{
		const auto result = empty(
			descriptor,
			memory_resource_affinity::host,
			context
		);
		result.get_access_hazard_tracker()->collect(read_only, tokens);
	}

	SECTION("when the array is written into an output without storage")
	{
		array out;
		empty(descriptor, memory_resource_affinity::host, context, &out);
		out.get_access_hazard_tracker()->collect(read_only, tokens);
	}

	REQUIRE( tokens.size() == 1 );
	CHECK( tokens[0].get_timeline() == timeline );
	CHECK( tokens[0].get_id() == 5u );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty routes the allocation to the device allocator for device affinity",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	// Only the device allocator is expected to be used; an unexpected call on
	// the host allocator would fail the test.
	ALLOW_CALL(*device_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*device_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	const auto result = empty(
		descriptor,
		memory_resource_affinity::device,
		context
	);

	CHECK( result.get_storage() == buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty reuses the storage of a compatible output array instead of "
	"allocating a new buffer",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	// The output buffer lives on the same resource as the allocator and is
	// large enough, so it should be reused verbatim (no allocate() call).
	ALLOW_CALL(*buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*buffer, get_size())
		.RETURN(size);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);

	array out(buffer, descriptor);

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context,
		&out
	);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( out.get_storage() == buffer.get() );

	// The descriptor already matched, so the output array is left untouched.
	CHECK( out.get_descriptor() == descriptor );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty reuses compatible storage but overrides the output array when its "
	"descriptor differs",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto stale_descriptor = make_descriptor({ 1 });
	const auto buffer = std::make_shared<mock_buffer>();

	// Same resource and large enough, so the buffer is reused, but the stale
	// descriptor must be replaced by the requested one.
	ALLOW_CALL(*buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*buffer, get_size())
		.RETURN(size);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);

	array out(buffer, stale_descriptor);
	const auto *tracker = out.get_access_hazard_tracker();

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context,
		&out
	);

	// The storage is reused (no allocation), but the descriptor is overridden.
	CHECK( result.get_storage() == buffer.get() );
	CHECK( out.get_storage() == buffer.get() );
	CHECK( out.get_descriptor() == descriptor );

	// The memory is the same, so the commands recorded on it still count.
	CHECK( out.get_access_hazard_tracker() == tracker );
	CHECK( result.get_access_hazard_tracker() == tracker );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty allocates and overrides the output array when it has no storage",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	// A default-constructed output has no storage to reuse.
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	array out;

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context,
		&out
	);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( out.get_storage() == buffer.get() );
	CHECK( out.get_descriptor() == descriptor );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty reallocates and overrides the output array when its storage lives "
	"on a different memory resource",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto stale_buffer = std::make_shared<mock_buffer>();
	const auto buffer = std::make_shared<mock_buffer>();

	// The stale buffer lives on the device resource, while the host allocator
	// hands out host-resource storage; reuse must be rejected.
	ALLOW_CALL(*stale_buffer, get_memory_resource())
		.LR_RETURN(device_resource);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	array out(stale_buffer, descriptor);

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context,
		&out
	);

	// A fresh buffer is allocated and the output array is overridden with it.
	CHECK( result.get_storage() == buffer.get() );
	CHECK( out.get_storage() == buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"empty reallocates and overrides the output array when its storage is too "
	"small",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto stale_buffer = std::make_shared<mock_buffer>();
	const auto buffer = std::make_shared<mock_buffer>();

	// The stale buffer is on the right resource but undersized; reuse must be
	// rejected in favor of a fresh allocation.
	ALLOW_CALL(*stale_buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*stale_buffer, get_size())
		.RETURN(size - 1);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	array out(stale_buffer, descriptor);

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context,
		&out
	);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( out.get_storage() == buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"full allocates an output array and dispatches a fill_operation carrying "
	"the requested value",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fill_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 0 )
		.LR_WITH( storage_of(_2, 0) == buffer.get() )
		.LR_WITH( in_session(_4) )
		.WITH( fills_with(_1, 2.5f) );

	const auto result = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(2.5f),
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"zeros dispatches a fill_operation with a zero fill value",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();

	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(_, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fill_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 0 )
		.WITH( fills_with(_1, 0) );

	const auto result = zeros(
		descriptor,
		memory_resource_affinity::host,
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ones dispatches a fill_operation with a one fill value",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();

	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(_, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fill_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 0 )
		.WITH( fills_with(_1, 1) );

	const auto result = ones(
		descriptor,
		memory_resource_affinity::host,
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"copy dispatches a copy_operation with the source as the single input",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto source_buffer = std::make_shared<mock_buffer>();
	array source(source_buffer, descriptor);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<copy_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( storage_of(_3, 0) == source_buffer.get() )
		.LR_WITH( in_session(_4) );

	const auto result = copy(source.share(), context);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fill dispatches a fill_operation on the provided array with no inputs",
	"[array_creation]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();
	array target(buffer, descriptor);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fill_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 0 )
		.LR_WITH( storage_of(_2, 0) == buffer.get() )
		.WITH( fills_with(_1, 7) );

	fill(target, scalar_value(7), context);
}
