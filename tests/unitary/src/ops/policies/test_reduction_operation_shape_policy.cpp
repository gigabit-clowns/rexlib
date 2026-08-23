// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/ops/policies/reduction_operation_shape_policy.hpp>

#include <xmipp4/core/dispatch/operand_names.hpp>
#include <xmipp4/core/dispatch/operation_descriptor.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;

namespace
{

using shape_type = operation_shape_policy::shape_type;

const operation_descriptor& reduction_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("result");
	static REX_CONST_CONSTEXPR auto inputs =
		make_operand_names("value");
	static const operation_descriptor instance =
		make_operation_descriptor("xmipp4.test", "reduce", outputs, inputs);
	return instance;
}

shape_type deduce_one(
	const reduction_operation_shape_policy &policy,
	const std::vector<shape_type> &inputs
)
{
	std::vector<shape_type> outputs(1);
	policy.deduce(
		reduction_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0];
}

} // anonymous namespace

TEST_CASE(
	"reduction_operation_shape_policy should drop the reduced axes",
	"[reduction_operation_shape_policy]"
)
{
	const reduction_operation_shape_policy policy({ 1 });
	CHECK( deduce_one(policy, { shape_type{ 2, 3, 4 } }) ==
	       shape_type{ 2, 4 } );
}

TEST_CASE(
	"reduction_operation_shape_policy should keep the reduced axes when "
	"asked",
	"[reduction_operation_shape_policy]"
)
{
	const reduction_operation_shape_policy policy({ 1 }, true);
	CHECK( deduce_one(policy, { shape_type{ 2, 3, 4 } }) ==
	       shape_type{ 2, 1, 4 } );
}

TEST_CASE(
	"reduction_operation_shape_policy should drop several axes at once",
	"[reduction_operation_shape_policy]"
)
{
	// Given out of order, to show that the policy does not depend on the
	// caller having sorted them.
	const reduction_operation_shape_policy policy({ 2, 0 });
	CHECK( deduce_one(policy, { shape_type{ 2, 3, 4, 5 } }) ==
	       shape_type{ 3, 5 } );

	const reduction_operation_shape_policy keeping({ 2, 0 }, true);
	CHECK( deduce_one(keeping, { shape_type{ 2, 3, 4, 5 } }) ==
	       shape_type{ 1, 3, 1, 5 } );
}

TEST_CASE(
	"reduction_operation_shape_policy should reduce over every axis when "
	"given every axis",
	"[reduction_operation_shape_policy]"
)
{
	// Reducing over all of them is not a mode of the policy: the caller
	// spells the axes out, so a full reduction is just the full list.
	const reduction_operation_shape_policy policy({ 0, 1, 2 });
	CHECK( deduce_one(policy, { shape_type{ 2, 3, 4 } }) == shape_type{} );
}

TEST_CASE(
	"reduction_operation_shape_policy should leave the shape alone when no "
	"axis is reduced",
	"[reduction_operation_shape_policy]"
)
{
	// An empty list reduces nothing, and must not be mistaken for a
	// request to reduce everything.
	const reduction_operation_shape_policy policy{
		reduction_operation_shape_policy::axis_list_type()
	};
	CHECK( deduce_one(policy, { shape_type{ 2, 3, 4 } }) ==
	       shape_type{ 2, 3, 4 } );
}

TEST_CASE(
	"reduction_operation_shape_policy should sort the axes it is given",
	"[reduction_operation_shape_policy]"
)
{
	const reduction_operation_shape_policy policy({ 3, 1, 2 });
	const auto axes = policy.get_axes();

	REQUIRE( axes.size() == 3 );
	CHECK( axes[0] == 1 );
	CHECK( axes[1] == 2 );
	CHECK( axes[2] == 3 );
	CHECK( policy.get_keep_dimensions() == false );
}

TEST_CASE(
	"reduction_operation_shape_policy should reject a repeated axis",
	"[reduction_operation_shape_policy]"
)
{
	// Reducing one axis twice cannot mean anything, and silently dropping
	// the duplicate would give a shape the caller did not ask for.
	CHECK_THROWS_AS(
		reduction_operation_shape_policy({ 1, 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"reduction_operation_shape_policy should reject an axis beyond the rank",
	"[reduction_operation_shape_policy]"
)
{
	// The rank is only known when the operands arrive, so this cannot be
	// caught at construction.
	const reduction_operation_shape_policy policy({ 5 });
	CHECK_THROWS_AS(
		deduce_one(policy, { shape_type{ 2, 3 } }),
		std::out_of_range
	);
}

TEST_CASE(
	"reduction_operation_shape_policy should broadcast its inputs before "
	"reducing",
	"[reduction_operation_shape_policy]"
)
{
	// Broadcasting first is what lets one policy serve a reduction taking
	// a mask alongside the operand being reduced.
	const reduction_operation_shape_policy policy({ 0 });
	CHECK( deduce_one(policy, { shape_type{ 1, 3 }, shape_type{ 4, 1 } }) ==
	       shape_type{ 3 } );
}

TEST_CASE(
	"reduction_operation_shape_policy should reject having no input",
	"[reduction_operation_shape_policy]"
)
{
	const reduction_operation_shape_policy policy({ 0 });
	CHECK_THROWS_AS( deduce_one(policy, {}), std::invalid_argument );
}
