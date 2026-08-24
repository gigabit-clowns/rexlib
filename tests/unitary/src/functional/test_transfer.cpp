// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/transfer.hpp>

#include <rexlib/ops/assignment/copy_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using trompeloeil::_;
using rexlib::test::dispatch_record;
using rexlib::test::verb_dispatch_fixture;


//
// transfer
//

TEST_CASE(
	"transfer throws when the input array has no associated storage",
	"[array_transfer]"
)
{
	// A default-constructed array carries no storage.
	array input;

	// The context is irrelevant here: the storage check fails first.
	const execution_context context;

	CHECK_THROWS_AS(
		transfer(input, memory_resource_affinity::host, context),
		std::invalid_argument
	);
}

TEST_CASE(
	"transfer throws when the execution context has no allocator for the "
	"requested affinity",
	"[array_transfer]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const array_descriptor descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		numerical_type::float32
	);
	const auto buffer = std::make_shared<mock_buffer>();
	array input(buffer, descriptor);

	// A default-constructed context is empty: it has no allocators.
	const execution_context context;

	CHECK_THROWS_AS(
		transfer(input, memory_resource_affinity::host, context),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"transfer aliases the input when it already lives on the target memory "
	"resource",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();

	// Both the input storage and the target allocator report the same memory
	// resource, so the input must be aliased verbatim: no allocation and no
	// copy dispatch are expected (an unexpected call would fail the test).
	ALLOW_CALL(*buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);

	array input(buffer, descriptor);

	const auto result = transfer(
		input,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor() == descriptor );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"transfer copies the input when it lives on a different memory resource",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto target_buffer = std::make_shared<mock_buffer>();

	// The input lives on the device resource while the host allocator hands out
	// host-resource storage, so aliasing is rejected and a copy is performed
	// via a fresh allocation on the host allocator.
	ALLOW_CALL(*source_buffer, get_memory_resource())
		.LR_RETURN(device_resource);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get())
	)
		.RETURN(target_buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	array input(source_buffer, descriptor);

	const auto result = transfer(
		input,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_storage() == target_buffer.get() );
	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.num_outputs == 1 );
	CHECK( record.num_inputs == 1 );
	CHECK( record.get_output_storage(0) == target_buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
	CHECK( record.session == session.get() );
}


//
// transfer_copy
//

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"transfer_copy allocates on the target allocator and dispatches a "
	"copy_operation with the input as the single source",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto target_buffer = std::make_shared<mock_buffer>();

	// A forced copy always allocates fresh storage on the target allocator.
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get())
	)
		.RETURN(target_buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	array source(source_buffer, descriptor);

	const auto result = transfer_copy(
		source.share(),
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_storage() == target_buffer.get() );
	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.num_outputs == 1 );
	CHECK( record.num_inputs == 1 );
	CHECK( record.get_output_storage(0) == target_buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
	CHECK( record.session == session.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"transfer_copy reuses the storage of the provided output array",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto out_buffer = std::make_shared<mock_buffer>();

	// The output buffer lives on the target resource and is large enough, so it
	// must be reused verbatim: no allocate() call is expected.
	ALLOW_CALL(*out_buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*out_buffer, get_size())
		.RETURN(size);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	array source(source_buffer, descriptor);
	array out(out_buffer, descriptor);

	const auto result = transfer_copy(
		source.share(),
		memory_resource_affinity::host,
		context,
		&out
	);

	// The output storage is reused and the copy dispatched into it.
	CHECK( result.get_storage() == out_buffer.get() );
	CHECK( out.get_storage() == out_buffer.get() );
	CHECK( record.called );
	CHECK( record.get_output_storage(0) == out_buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
}


//
// to_device / to_host / to_device_copy / to_host_copy wrappers
//

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"to_device transfers using the device affinity",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();

	// The input lives on the device resource. Aliasing therefore proves that
	// to_device consulted the device allocator (a host allocator would report a
	// different resource and force a copy instead).
	ALLOW_CALL(*buffer, get_memory_resource())
		.LR_RETURN(device_resource);
	ALLOW_CALL(*device_allocator, get_memory_resource())
		.LR_RETURN(device_resource);

	array input(buffer, descriptor);

	const auto result = to_device(input, context);

	CHECK( result.get_storage() == buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"to_host transfers using the host affinity",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto buffer = std::make_shared<mock_buffer>();

	// Aliasing against the host resource proves the host allocator was used.
	ALLOW_CALL(*buffer, get_memory_resource())
		.LR_RETURN(host_resource);
	ALLOW_CALL(*host_allocator, get_memory_resource())
		.LR_RETURN(host_resource);

	array input(buffer, descriptor);

	const auto result = to_host(input, context);

	CHECK( result.get_storage() == buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"to_device_copy allocates on the device allocator",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto device_buffer = std::make_shared<mock_buffer>();

	// Only the device allocator is expected to be used.
	ALLOW_CALL(*device_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*device_allocator,
		allocate(size, _, default_queue.get())
	)
		.RETURN(device_buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	array source(source_buffer, descriptor);

	const auto result = to_device_copy(source.share(), context);

	CHECK( result.get_storage() == device_buffer.get() );
	CHECK( record.called );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"to_host_copy allocates on the host allocator",
	"[array_transfer]"
)
{
	const auto descriptor = make_descriptor();
	const auto size = compute_storage_requirement(descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto host_buffer = std::make_shared<mock_buffer>();

	// Only the host allocator is expected to be used.
	ALLOW_CALL(*host_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*host_allocator,
		allocate(size, _, default_queue.get())
	)
		.RETURN(host_buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	array source(source_buffer, descriptor);

	const auto result = to_host_copy(source.share(), context);

	CHECK( result.get_storage() == host_buffer.get() );
	CHECK( record.called );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
}
