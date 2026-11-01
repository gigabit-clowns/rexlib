// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/random.hpp>

#include <rexlib/ops/assignment/fill_operation.hpp>
#include <rexlib/ops/random/fold_in_key_operation.hpp>
#include <rexlib/ops/random/split_key_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

using namespace rexlib;
using namespace rexlib::ops;
using trompeloeil::_;
using rexlib::test::verb_dispatch_fixture;

namespace
{

/**
 * @brief Whether @p op is a fill_operation carrying @p value.
 */
bool fills_with(const operation &op, std::uint64_t value)
{
	const auto *fill = dynamic_cast<const fill_operation*>(&op);
	return fill && fill->get_fill_value().get<std::uint64_t>() == value;
}

/**
 * @brief Whether @p op is a split_key_operation carrying @p count.
 */
bool splits_into(const operation &op, std::size_t count)
{
	const auto *split = dynamic_cast<const split_key_operation*>(&op);
	return split && split->get_count() == count;
}

} // anonymous namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"make_key fills a uint64 array without axes in device memory with the "
	"seed",
	"[random]"
)
{
	const std::uint64_t seed = 0x0123456789abcdefu;
	const auto descriptor = make_descriptor({}, numerical_type::uint64);
	const auto size = compute_storage_requirement(descriptor);
	const auto buffer = std::make_shared<mock_buffer>();

	ALLOW_CALL(*device_allocator, get_max_alignment())
		.RETURN(std::size_t(256));
	REQUIRE_CALL(
		*device_allocator,
		allocate(size, _, default_queue.get(), _)
	)
		.RETURN(buffer);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fill_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 0 )
		.LR_WITH( storage_of(_2, 0) == buffer.get() )
		.LR_WITH( in_session(_4) )
		.WITH( fills_with(_1, seed) );

	const auto key = make_key(seed, context);

	CHECK( key.get_storage() == buffer.get() );
	CHECK( key.get_descriptor() == descriptor );
}

TEST_CASE(
	"entropy_seed draws a different seed on every call",
	"[random]"
)
{
	const auto first = entropy_seed();
	const auto second = entropy_seed();
	const auto third = entropy_seed();

	CHECK( first != second );
	CHECK( second != third );
	CHECK( ((first | second | third) >> 32) != 0 );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fold_in_key dispatches a fold_in_key_operation with the key and the "
	"data in order",
	"[random]"
)
{
	check_binary_verb<fold_in_key_operation>(
		fold_in_key,
		numerical_type::uint64
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"split_key dispatches a split_key_operation carrying the count",
	"[random]"
)
{
	std::shared_ptr<mock_buffer> key_storage;
	const array key = make_operand(key_storage, numerical_type::uint64);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<split_key_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == key_storage.get() )
		.WITH( splits_into(_1, 3) );

	split_key(key, 3, context);
}
