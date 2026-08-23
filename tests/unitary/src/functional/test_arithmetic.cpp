// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/arithmetic.hpp>

#include <rex/ops/arithmetic/abs_operation.hpp>
#include <rex/ops/arithmetic/add_operation.hpp>
#include <rex/ops/arithmetic/divide_operation.hpp>
#include <rex/ops/arithmetic/divmod_operation.hpp>
#include <rex/ops/arithmetic/floor_divide_operation.hpp>
#include <rex/ops/arithmetic/modulo_operation.hpp>
#include <rex/ops/arithmetic/multiply_operation.hpp>
#include <rex/ops/arithmetic/negate_operation.hpp>
#include <rex/ops/arithmetic/sign_operation.hpp>
#include <rex/ops/arithmetic/subtract_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;
using trompeloeil::_;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"add dispatches an add_operation with both inputs in order",
	"[array_arithmetic]"
)
{
	check_binary_verb<add_operation>(add);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"subtract dispatches a subtract_operation with both inputs in order",
	"[array_arithmetic]"
)
{
	check_binary_verb<subtract_operation>(subtract);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"multiply dispatches a multiply_operation with both inputs in order",
	"[array_arithmetic]"
)
{
	check_binary_verb<multiply_operation>(multiply);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"divide dispatches a divide_operation with both inputs in order",
	"[array_arithmetic]"
)
{
	check_binary_verb<divide_operation>(divide);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"modulo dispatches a modulo_operation with both inputs in order",
	"[array_arithmetic]"
)
{
	check_binary_verb<modulo_operation>(modulo);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"negate dispatches a negate_operation with its single input",
	"[array_arithmetic]"
)
{
	check_unary_verb<negate_operation>(negate);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"abs dispatches an abs_operation with its single input",
	"[array_arithmetic]"
)
{
	check_unary_verb<abs_operation>(abs);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"floor_divide dispatches a floor_divide_operation with both inputs in "
	"order",
	"[array_arithmetic]"
)
{
	check_binary_verb<floor_divide_operation>(floor_divide);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sign dispatches a sign_operation with its single input",
	"[array_arithmetic]"
)
{
	check_unary_verb<sign_operation>(sign);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"divmod dispatches a divmod_operation with two outputs",
	"[array_arithmetic]"
)
{
	// The only verb in the catalog with more than one output, so this is
	// what proves the multi-output path reaches the dispatcher intact.
	std::shared_ptr<mock_buffer> x_storage;
	std::shared_ptr<mock_buffer> y_storage;
	const array x_array = make_operand(x_storage, numerical_type::int32);
	const array y_array = make_operand(y_storage, numerical_type::int32);
	const const_array_ref x = x_array;
	const const_array_ref y = y_array;

	test::dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	divmod(x, y, context, nullptr, nullptr);

	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(divmod_operation) );
	CHECK( record.num_outputs == 2 );
	CHECK( record.num_inputs == 2 );
	CHECK( record.get_input_storage(0) == x_storage.get() );
	CHECK( record.get_input_storage(1) == y_storage.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"divmod reuses the storage of both pre-allocated outputs",
	"[array_arithmetic]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	std::shared_ptr<mock_buffer> y_storage;
	std::shared_ptr<mock_buffer> quotient_storage;
	std::shared_ptr<mock_buffer> remainder_storage;
	const array x_array = make_operand(x_storage, numerical_type::int32);
	const array y_array = make_operand(y_storage, numerical_type::int32);
	array quotient = make_operand(quotient_storage, numerical_type::int32);
	array remainder = make_operand(remainder_storage, numerical_type::int32);
	const const_array_ref x = x_array;
	const const_array_ref y = y_array;

	test::dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	const auto result = divmod(x, y, context, &quotient, &remainder);

	// Each output has to land in its own slot, in signature order. Getting
	// the two the wrong way round is the mistake a single output verb
	// cannot make.
	CHECK( record.get_output_storage(0) == quotient_storage.get() );
	CHECK( record.get_output_storage(1) == remainder_storage.get() );
	CHECK( result.first.get_storage() == quotient_storage.get() );
	CHECK( result.second.get_storage() == remainder_storage.get() );
}
