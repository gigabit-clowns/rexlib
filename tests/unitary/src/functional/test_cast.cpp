// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/cast.hpp>

#include <rexlib/ops/assignment/copy_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using trompeloeil::_;
using rexlib::test::verb_dispatch_fixture;


TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cast returns an alias of the input without allocating or dispatching when "
	"the array already has the requested type",
	"[array_cast]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);
	const auto buffer = std::make_shared<mock_buffer>();
	array input(buffer, descriptor);

	// Neither the allocators nor the dispatcher are expected to be used; an
	// unexpected call on any of them would fail the test.
	const auto result = cast(input, numerical_type::float32, context);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor() == descriptor );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cast allocates a new device array and dispatches a copy_operation when the "
	"requested type differs",
	"[array_cast]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto buffer = std::make_shared<mock_buffer>();
	array input(source_buffer, descriptor);

	ALLOW_CALL(*device_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*device_allocator,
		allocate(_, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<copy_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( storage_of(_2, 0) == buffer.get() )
		.LR_WITH( storage_of(_3, 0) == source_buffer.get() )
		.LR_WITH( in_session(_4) );

	const auto result = cast(input, numerical_type::float64, context);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cast_copy allocates a new device array and dispatches a copy_operation "
	"even when the input already has the requested type",
	"[array_cast]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto buffer = std::make_shared<mock_buffer>();
	array input(source_buffer, descriptor);

	ALLOW_CALL(*device_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*device_allocator,
		allocate(_, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<copy_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( storage_of(_2, 0) == buffer.get() )
		.LR_WITH( storage_of(_3, 0) == source_buffer.get() )
		.LR_WITH( in_session(_4) );

	const auto result = cast_copy(input.share(), numerical_type::float32, context);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float32 );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cast_copy casts to the requested type and reuses a compatible output array",
	"[array_cast]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);
	const auto target_descriptor =
		make_descriptor({ 2, 3 }, numerical_type::float64);
	const auto size = compute_storage_requirement(target_descriptor);
	const auto source_buffer = std::make_shared<mock_buffer>();
	const auto out_buffer = std::make_shared<mock_buffer>();
	array input(source_buffer, descriptor);

	// The output buffer lives on the device resource and is large enough, so it
	// is reused verbatim (no allocate() call).
	ALLOW_CALL(*out_buffer, get_memory_resource())
		.LR_RETURN(device_resource);
	ALLOW_CALL(*out_buffer, get_size())
		.RETURN(size);
	ALLOW_CALL(*device_allocator, get_memory_resource())
		.LR_RETURN(device_resource);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<copy_operation>(_1) )
		.LR_WITH( storage_of(_2, 0) == out_buffer.get() )
		.LR_WITH( storage_of(_3, 0) == source_buffer.get() );

	array out(out_buffer, target_descriptor);

	const auto result = cast_copy(
		input.share(),
		numerical_type::float64,
		context,
		&out
	);

	CHECK( result.get_storage() == out_buffer.get() );
	CHECK( out.get_storage() == out_buffer.get() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );
}
