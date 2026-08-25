// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/trigonometric.hpp>

#include <rexlib/ops/trigonometric/acos_operation.hpp>
#include <rexlib/ops/trigonometric/acosh_operation.hpp>
#include <rexlib/ops/trigonometric/asin_operation.hpp>
#include <rexlib/ops/trigonometric/asinh_operation.hpp>
#include <rexlib/ops/trigonometric/atan_operation.hpp>
#include <rexlib/ops/trigonometric/atan2_operation.hpp>
#include <rexlib/ops/trigonometric/atanh_operation.hpp>
#include <rexlib/ops/trigonometric/cos_operation.hpp>
#include <rexlib/ops/trigonometric/cosh_operation.hpp>
#include <rexlib/ops/trigonometric/degrees_operation.hpp>
#include <rexlib/ops/trigonometric/hypot_operation.hpp>
#include <rexlib/ops/trigonometric/radians_operation.hpp>
#include <rexlib/ops/trigonometric/sin_operation.hpp>
#include <rexlib/ops/trigonometric/sincos_operation.hpp>
#include <rexlib/ops/trigonometric/sinh_operation.hpp>
#include <rexlib/ops/trigonometric/tan_operation.hpp>
#include <rexlib/ops/trigonometric/tanh_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::verb_dispatch_fixture;
using trompeloeil::_;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sin dispatches a sin_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<sin_operation>(sin);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cos dispatches a cos_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<cos_operation>(cos);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"tan dispatches a tan_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<tan_operation>(tan);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"asin dispatches a asin_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<asin_operation>(asin);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"acos dispatches a acos_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<acos_operation>(acos);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"atan dispatches a atan_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<atan_operation>(atan);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sinh dispatches a sinh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<sinh_operation>(sinh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"cosh dispatches a cosh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<cosh_operation>(cosh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"tanh dispatches a tanh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<tanh_operation>(tanh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"asinh dispatches a asinh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<asinh_operation>(asinh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"acosh dispatches a acosh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<acosh_operation>(acosh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"atanh dispatches a atanh_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<atanh_operation>(atanh);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"atan2 dispatches a atan2_operation with both inputs in order",
	"[array_trigonometric]"
)
{
	check_binary_verb<atan2_operation>(atan2);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"hypot dispatches a hypot_operation with both inputs in order",
	"[array_trigonometric]"
)
{
	check_binary_verb<hypot_operation>(hypot);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"degrees dispatches a degrees_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<degrees_operation>(degrees);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"radians dispatches a radians_operation with its single input",
	"[array_trigonometric]"
)
{
	check_unary_verb<radians_operation>(radians);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sincos dispatches a sincos_operation with two outputs and one input",
	"[array_trigonometric]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	std::shared_ptr<mock_buffer> sine_storage;
	std::shared_ptr<mock_buffer> cosine_storage;
	const array x_array = make_operand(x_storage);
	array sine = make_operand(sine_storage);
	array cosine = make_operand(cosine_storage);
	const const_array_ref x = x_array;

	test::dispatch_record record;
	REQUIRE_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	const auto result = sincos(x, context, &sine, &cosine);

	CHECK( record.called );
	REQUIRE( record.operation_type != nullptr );
	CHECK( *record.operation_type == typeid(sincos_operation) );
	CHECK( record.num_outputs == 2 );
	CHECK( record.num_inputs == 1 );
	CHECK( record.get_input_storage(0) == x_storage.get() );
	CHECK( record.get_output_storage(0) == sine_storage.get() );
	CHECK( record.get_output_storage(1) == cosine_storage.get() );
	CHECK( result.first.get_storage() == sine_storage.get() );
	CHECK( result.second.get_storage() == cosine_storage.get() );
}
