// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/linalg.hpp>

#include <rexlib/ops/linalg/cross_operation.hpp>
#include <rexlib/ops/linalg/matmul_operation.hpp>
#include <rexlib/ops/linalg/matvec_operation.hpp>
#include <rexlib/ops/linalg/vecdot_operation.hpp>
#include <rexlib/ops/linalg/vecmat_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::verb_dispatch_fixture;
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

	// The operands are two dimensional, so the last axis is axis 1.
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<vecdot_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 2 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == x_storage.get() )
		.LR_WITH( storage_of(_3, 1) == y_storage.get() )
		.WITH(
			axes_of<vecdot_operation>(_1) ==
				std::vector<std::size_t>{ 1 }
		);

	vecdot(x, y, context, nullptr);
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

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<cross_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 2 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<cross_operation>(_1) == std::vector<std::size_t>{ 1 }
		);

	cross(x, y, -1, context, nullptr);
}
