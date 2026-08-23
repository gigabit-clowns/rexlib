// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/cast.hpp>

#include <xmipp4/ops/assignment/copy_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using trompeloeil::_;
using xmipp4::test::dispatch_record;
using xmipp4::test::verb_dispatch_fixture;


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
		allocate(_, _, default_queue.get())
	)
		.RETURN(buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	const auto result = cast(input, numerical_type::float64, context);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );

	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.num_outputs == 1 );
	CHECK( record.num_inputs == 1 );
	CHECK( record.get_output_storage(0) == buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
	CHECK( record.session == session.get() );
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
		allocate(_, _, default_queue.get())
	)
		.RETURN(buffer);

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	const auto result = cast_copy(input.share(), numerical_type::float32, context);

	CHECK( result.get_storage() == buffer.get() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float32 );

	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.num_outputs == 1 );
	CHECK( record.num_inputs == 1 );
	CHECK( record.get_output_storage(0) == buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
	CHECK( record.session == session.get() );
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

	dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

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

	CHECK( record.called );
	CHECK( *record.operation_type == typeid(copy_operation) );
	CHECK( record.get_output_storage(0) == out_buffer.get() );
	CHECK( record.get_input_storage(0) == source_buffer.get() );
}
