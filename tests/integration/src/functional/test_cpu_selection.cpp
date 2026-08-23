// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/functional/selection.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <xmipp4/ops/selection/clip_operation.hpp>
#include <xmipp4/ops/selection/maximum_operation.hpp>
#include <xmipp4/ops/selection/minimum_operation.hpp>
#include <xmipp4/ops/selection/where_operation.hpp>

#include <cmath>
#include <limits>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::element_value;
using xmipp4::test::elementwise_verb_fixture;

namespace
{

const double not_a_number = std::numeric_limits<double>::quiet_NaN();

} // anonymous namespace

// The operands are zero and one wherever a case covers the whole domain,
// booleans being among the types admitted and holding only those two.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"where takes the second operand where the condition holds",
	"[array_selection][cpu]"
)
{
	// The condition is boolean whatever is being selected between, so the
	// driver builds it from the rule rather than from the swept type.
	check_ternary<where_operation>(
		xmipp4::where,
		element_value(1),
		element_value(0),
		element_value(1),
		[](auto, auto, auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"where takes the third operand where the condition fails",
	"[array_selection][cpu]"
)
{
	check_ternary<where_operation>(
		xmipp4::where,
		element_value(0),
		element_value(0),
		element_value(1),
		[](auto, auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"clip leaves an element between the bounds alone",
	"[array_selection][cpu]"
)
{
	check_ternary<clip_operation>(
		xmipp4::clip,
		element_value(2),
		element_value(0),
		element_value(3),
		[](auto, auto, auto) { return 2; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"clip brings an element above the upper bound down to it",
	"[array_selection][cpu]"
)
{
	check_ternary<clip_operation>(
		xmipp4::clip,
		element_value(5),
		element_value(0),
		element_value(3),
		[](auto, auto, auto) { return 3; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"clip brings an element below the lower bound up to it",
	"[array_selection][cpu]"
)
{
	check_ternary<clip_operation>(
		xmipp4::clip,
		element_value(0),
		element_value(1),
		element_value(3),
		[](auto, auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"clip with bounds that cross leaves the upper one winning",
	"[array_selection][cpu]"
)
{
	// Applying the lower bound first and the upper second, as NumPy does,
	// rather than reporting the bounds as an error.
	check_ternary<clip_operation>(
		xmipp4::clip,
		element_value(5),
		element_value(3),
		element_value(1),
		[](auto, auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"maximum takes the larger of each pair",
	"[array_selection][cpu]"
)
{
	check_binary<maximum_operation>(
		xmipp4::maximum,
		element_value(0),
		element_value(1),
		[](auto, auto) { return 1; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"minimum takes the smaller of each pair",
	"[array_selection][cpu]"
)
{
	check_binary<minimum_operation>(
		xmipp4::minimum,
		element_value(0),
		element_value(1),
		[](auto, auto) { return 0; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"maximum and minimum propagate an unordered element from either side",
	"[array_selection][cpu]"
)
{
	// Spelled out rather than driven, because the driver compares with
	// equality and no not-a-number satisfies that.
	//
	// std::max would propagate it from one side only and std::fmax from
	// neither, either of which would leave the operation answering
	// differently depending on the order of its operands.
	auto ordinary = make_operand<float32_t>(element_value(1));
	auto unordered = make_operand<float32_t>(element_value(not_a_number));
	const const_array_ref ordinary_ref = ordinary;
	const const_array_ref unordered_ref = unordered;

	const auto larger_first =
		xmipp4::maximum(unordered_ref, ordinary_ref, context, nullptr);
	const auto larger_second =
		xmipp4::maximum(ordinary_ref, unordered_ref, context, nullptr);
	const auto smaller_first =
		xmipp4::minimum(unordered_ref, ordinary_ref, context, nullptr);
	const auto smaller_second =
		xmipp4::minimum(ordinary_ref, unordered_ref, context, nullptr);

	for (const auto value : read_host<float32_t>(larger_first, 6))
	{
		CHECK( std::isnan(value) );
	}
	for (const auto value : read_host<float32_t>(larger_second, 6))
	{
		CHECK( std::isnan(value) );
	}
	for (const auto value : read_host<float32_t>(smaller_first, 6))
	{
		CHECK( std::isnan(value) );
	}
	for (const auto value : read_host<float32_t>(smaller_second, 6))
	{
		CHECK( std::isnan(value) );
	}
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"clip propagates an unordered element through both bounds",
	"[array_selection][cpu]"
)
{
	auto unordered = make_operand<float32_t>(element_value(not_a_number));
	auto lower = make_operand<float32_t>(element_value(0));
	auto upper = make_operand<float32_t>(element_value(1));
	const const_array_ref unordered_ref = unordered;
	const const_array_ref lower_ref = lower;
	const const_array_ref upper_ref = upper;

	const auto result = xmipp4::clip(
		unordered_ref, lower_ref, upper_ref, context, nullptr
	);

	for (const auto value : read_host<float32_t>(result, 6))
	{
		CHECK( std::isnan(value) );
	}
}
