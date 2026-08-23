// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/ops/policies/sequence_operation_shape_policy.hpp>

#include <rex/core/dispatch/operand_names.hpp>
#include <rex/core/dispatch/operation_descriptor.hpp>
#include <rex/core/platform/constexpr.hpp>

#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;

namespace
{

using shape_type = operation_shape_policy::shape_type;

const operation_descriptor& sequence_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("result");
	static REX_CONST_CONSTEXPR auto inputs =
		make_operand_names();
	static const operation_descriptor instance =
		make_operation_descriptor("rex.test", "sequence", outputs, inputs);
	return instance;
}

shape_type deduce_one(
	const sequence_operation_shape_policy &policy,
	const std::vector<shape_type> &inputs
)
{
	std::vector<shape_type> outputs(1);
	policy.deduce(
		sequence_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0];
}

} // anonymous namespace


TEST_CASE(
	"sequence_operation_shape_policy should deduce a one dimensional shape "
	"of the length it holds",
	"[sequence_operation_shape_policy]"
)
{
	const sequence_operation_shape_policy policy(7);
	CHECK( policy.get_count() == 7 );
	CHECK( deduce_one(policy, {}) == shape_type{ 7 } );
}

TEST_CASE(
	"sequence_operation_shape_policy should admit an empty sequence",
	"[sequence_operation_shape_policy]"
)
{
	// Rank one with no element, rather than rank zero: an empty range is
	// still a range.
	const sequence_operation_shape_policy policy(0);
	CHECK( deduce_one(policy, {}) == shape_type{ 0 } );
}

TEST_CASE(
	"sequence_operation_shape_policy should give every output the same shape",
	"[sequence_operation_shape_policy]"
)
{
	const sequence_operation_shape_policy policy(3);

	std::vector<shape_type> outputs(2);
	policy.deduce(
		sequence_descriptor(),
		make_span(outputs),
		span<const shape_type>()
	);

	CHECK( outputs[0] == shape_type{ 3 } );
	CHECK( outputs[1] == shape_type{ 3 } );
}

TEST_CASE(
	"sequence_operation_shape_policy should reject an input operand",
	"[sequence_operation_shape_policy]"
)
{
	const sequence_operation_shape_policy policy(3);
	CHECK_THROWS_AS(
		deduce_one(policy, { shape_type{ 3 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"sequence_operation_shape_policy should require a user output to match "
	"the length exactly",
	"[sequence_operation_shape_policy]"
)
{
	// The inherited acceptance is equality, which is what a length that is a
	// parameter rather than a deduction calls for: there is nothing for a
	// differently shaped output to be broadcast from.
	const sequence_operation_shape_policy policy(4);
	const std::vector<shape_type> canonical = { shape_type{ 4 } };

	const std::vector<shape_type> matching = { shape_type{ 4 } };
	CHECK_NOTHROW(
		policy.accept(
			sequence_descriptor(),
			make_span(matching),
			make_span(canonical),
			span<const shape_type>()
		)
	);

	const std::vector<shape_type> mismatched = { shape_type{ 5 } };
	CHECK_THROWS_AS(
		policy.accept(
			sequence_descriptor(),
			make_span(mismatched),
			make_span(canonical),
			span<const shape_type>()
		),
		std::invalid_argument
	);
}
