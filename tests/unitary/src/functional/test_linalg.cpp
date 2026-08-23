// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/linalg.hpp>

#include <rex/ops/linalg/cross_operation.hpp>
#include <rex/ops/linalg/matmul_operation.hpp>
#include <rex/ops/linalg/matvec_operation.hpp>
#include <rex/ops/linalg/vecdot_operation.hpp>
#include <rex/ops/linalg/vecmat_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using rex::test::verb_dispatch_fixture;
using trompeloeil::_;

#include <cstddef>
#include <vector>

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"matmul dispatches a matmul_operation with both inputs in order",
	"[array_linalg]"
)
{
	check_binary_verb<matmul_operation>(matmul);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"matvec dispatches a matvec_operation with both inputs in order",
	"[array_linalg]"
)
{
	check_binary_verb<matvec_operation>(matvec);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"vecmat dispatches a vecmat_operation with both inputs in order",
	"[array_linalg]"
)
{
	check_binary_verb<vecmat_operation>(vecmat);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"vecdot contracts the last axis by default",
	"[array_linalg]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	std::shared_ptr<mock_buffer> y_storage;
	const array x_array = make_operand(x_storage);
	const array y_array = make_operand(y_storage);
	const const_array_ref x = x_array;
	const const_array_ref y = y_array;

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto axes = dynamic_cast<const vecdot_operation&>(op)
			.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	vecdot(x, y, context, nullptr);

	check_record<vecdot_operation>(record, 2);
	CHECK( record.get_input_storage(0) == x_storage.get() );
	CHECK( record.get_input_storage(1) == y_storage.get() );

	// The operands are two dimensional, so the last axis is axis 1.
	REQUIRE( seen.size() == 1 );
	CHECK( seen[0] == 1 );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cross resolves a negative axis against the broadcast rank",
	"[array_linalg]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	std::shared_ptr<mock_buffer> y_storage;
	const array x_array = make_operand(x_storage);
	const array y_array = make_operand(y_storage);
	const const_array_ref x = x_array;
	const const_array_ref y = y_array;

	test::dispatch_record record;
	std::size_t seen = 0;
	record.inspect = [&seen] (const operation &op)
	{
		seen = dynamic_cast<const cross_operation&>(op)
			.get_shape_policy().get_axis();
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	cross(x, y, -1, context, nullptr);

	check_record<cross_operation>(record, 2);
	CHECK( seen == 1 );
}
