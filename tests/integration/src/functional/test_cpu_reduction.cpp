// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rex/functional/reduction.hpp>

#include "fixtures/reduction_verb_fixture.hpp"

#include <rex/ops/reduction/all_operation.hpp>
#include <rex/ops/reduction/amax_operation.hpp>
#include <rex/ops/reduction/amin_operation.hpp>
#include <rex/ops/reduction/any_operation.hpp>
#include <rex/ops/reduction/argmax_operation.hpp>
#include <rex/ops/reduction/argmin_operation.hpp>
#include <rex/ops/reduction/count_nonzero_operation.hpp>
#include <rex/ops/reduction/mean_operation.hpp>
#include <rex/ops/reduction/product_operation.hpp>
#include <rex/ops/reduction/sum_operation.hpp>

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::element_value;
using xmipp4::test::reduction_verb_fixture;

namespace
{

const double not_a_number = std::numeric_limits<double>::quiet_NaN();

// The input is filled with one value, so a sum is that value repeated. Taking
// it as one keeps the answer inside the narrowest type the operation admits,
// which is what lets the whole domain be swept from a single case.
template <typename T>
T repeated(T value, std::size_t count) noexcept
{
	return value * static_cast<T>(count);
}

} // anonymous namespace

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum adds the elements along one axis",
	"[array_reduction][cpu]"
)
{
	// The last axis is the contiguous one, so this is the orientation whose
	// reduced elements are walked contiguously.
	check_reduction<sum_operation>(
		xmipp4::sum,
		{ 2 },
		false,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum adds the elements along an outer axis",
	"[array_reduction][cpu]"
)
{
	// The surviving axes are the contiguous ones here, which is the other
	// orientation of the loop and the one a naive per-output pass gets wrong.
	check_reduction<sum_operation>(
		xmipp4::sum,
		{ 0 },
		false,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum adds the elements along several axes at once",
	"[array_reduction][cpu]"
)
{
	check_reduction<sum_operation>(
		xmipp4::sum,
		{ 0, 2 },
		false,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum adds every element when every axis is reduced",
	"[array_reduction][cpu]"
)
{
	// Leaves an output of rank zero, which is the shape the surviving space
	// collapses to.
	check_reduction<sum_operation>(
		xmipp4::sum,
		{ 0, 1, 2 },
		false,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum leaves the array alone when no axis is reduced",
	"[array_reduction][cpu]"
)
{
	// An empty axis list is not a request to reduce everything, so this is a
	// copy rather than a total.
	check_reduction<sum_operation>(
		xmipp4::sum,
		{},
		false,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum keeps the reduced axes at one when asked to",
	"[array_reduction][cpu]"
)
{
	check_reduction<sum_operation>(
		xmipp4::sum,
		{ 1 },
		true,
		element_value(1),
		[](auto x, std::size_t count) { return repeated(x, count); }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"product multiplies the elements along an axis",
	"[array_reduction][cpu]"
)
{
	check_reduction<product_operation>(
		xmipp4::product,
		{ 1 },
		false,
		element_value(1),
		[](auto x, std::size_t) { return x; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"amax and amin carry a constant array through unchanged",
	"[array_reduction][cpu]"
)
{
	// Every element being equal, this pins the typing and the shape rather
	// than the choice itself, which the cases below make.
	check_reduction<amax_operation>(
		xmipp4::amax,
		{ 0, 2 },
		false,
		element_value(2),
		[](auto x, std::size_t) { return x; }
	);
	check_reduction<amin_operation>(
		xmipp4::amin,
		{ 0, 2 },
		false,
		element_value(2),
		[](auto x, std::size_t) { return x; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"product multiplies elements that differ",
	"[array_reduction][cpu]"
)
{
	// Two rows of three, reduced along the rows.
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<element_value> values = { 1, 2, 3, 4, 5, 6 };
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 0 };
	const auto result = xmipp4::product(
		operand_ref, make_span(axes), false, context, nullptr
	);

	check_values<float32_t>(result, { 3 }, { 4, 10, 18 });
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"amax takes the largest element and amin the smallest",
	"[array_reduction][cpu]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<element_value> values = { 5, 1, 9, 3, 7, 2 };
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	SECTION( "along the axis the elements are strided over" )
	{
		const std::vector<std::ptrdiff_t> axes = { 0 };
		check_values<float32_t>(
			xmipp4::amax(operand_ref, make_span(axes), false, context, nullptr),
			{ 3 },
			{ 5, 7, 9 }
		);
		check_values<float32_t>(
			xmipp4::amin(operand_ref, make_span(axes), false, context, nullptr),
			{ 3 },
			{ 3, 1, 2 }
		);
	}

	SECTION( "along the axis the elements are contiguous over" )
	{
		const std::vector<std::ptrdiff_t> axes = { 1 };
		check_values<float32_t>(
			xmipp4::amax(operand_ref, make_span(axes), false, context, nullptr),
			{ 2 },
			{ 9, 7 }
		);
		check_values<float32_t>(
			xmipp4::amin(operand_ref, make_span(axes), false, context, nullptr),
			{ 2 },
			{ 1, 2 }
		);
	}
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"amax and amin propagate an unordered element",
	"[array_reduction][cpu]"
)
{
	// std::max would propagate it only from one side, leaving the answer
	// dependent on where in the array it fell.
	const std::vector<std::size_t> extents = { 3 };
	const std::vector<std::ptrdiff_t> axes = { 0 };

	const auto values = GENERATE(
		std::vector<element_value>{ not_a_number, 1, 2 },
		std::vector<element_value>{ 1, not_a_number, 2 },
		std::vector<element_value>{ 1, 2, not_a_number }
	);

	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const auto largest =
		xmipp4::amax(operand_ref, make_span(axes), false, context, nullptr);
	const auto smallest =
		xmipp4::amin(operand_ref, make_span(axes), false, context, nullptr);

	CHECK( std::isnan(read_host<float32_t>(largest, 1).front()) );
	CHECK( std::isnan(read_host<float32_t>(smallest, 1).front()) );
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"amax and amin reject a reduction over no elements",
	"[array_reduction][cpu]"
)
{
	// There is no largest element of nothing, and the lowest representable
	// value would be an invention rather than an answer. NumPy refuses the
	// same way.
	auto operand = make_sequence_operand<float32_t>({ 0, 3 }, {});
	const const_array_ref operand_ref = operand;
	const std::vector<std::ptrdiff_t> axes = { 0 };

	CHECK_THROWS_AS(
		xmipp4::amax(operand_ref, make_span(axes), false, context, nullptr),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		xmipp4::amin(operand_ref, make_span(axes), false, context, nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"sum answers a reduction over no elements with zero",
	"[array_reduction][cpu]"
)
{
	// An empty sum has an answer where an empty extremum does not, which is
	// the whole of the difference between a fold with a neutral element and
	// one without.
	auto operand = make_sequence_operand<float32_t>({ 0, 3 }, {});
	const const_array_ref operand_ref = operand;
	const std::vector<std::ptrdiff_t> axes = { 0 };

	check_values<float32_t>(
		xmipp4::sum(operand_ref, make_span(axes), false, context, nullptr),
		{ 3 },
		{ 0, 0, 0 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"mean averages the elements along an axis",
	"[array_reduction][cpu]"
)
{
	// Every element being the same, the average is that element, whatever
	// the count. The division is pinned by the case below instead.
	check_reduction<mean_operation>(
		xmipp4::mean,
		{ 1 },
		false,
		element_value(2),
		[](auto x, std::size_t) { return x; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"mean divides the total by the number of elements",
	"[array_reduction][cpu]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<element_value> values = { 1, 2, 3, 4, 5, 6 };
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 1 };
	check_values<float32_t>(
		xmipp4::mean(operand_ref, make_span(axes), false, context, nullptr),
		{ 2 },
		{ 2, 5 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"mean of an integer array answers in an inexact type",
	"[array_reduction][cpu]"
)
{
	// The rule asks for the inexact counterpart of the operand type, so an
	// average that is not a whole number survives rather than truncating.
	auto operand = make_sequence_operand<int32_t>({ 2 }, { 1, 2 });
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 0 };
	check_values<float64_t>(
		xmipp4::mean(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 1.5 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"count_nonzero counts the elements that are set",
	"[array_reduction][cpu]"
)
{
	check_reduction<count_nonzero_operation>(
		xmipp4::count_nonzero,
		{ 0 },
		false,
		element_value(1),
		[](auto, std::size_t count) { return count; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"count_nonzero answers with zero for elements that are not set",
	"[array_reduction][cpu]"
)
{
	check_reduction<count_nonzero_operation>(
		xmipp4::count_nonzero,
		{ 0 },
		false,
		element_value(0),
		[](auto, std::size_t) { return 0; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"count_nonzero counts a mixture",
	"[array_reduction][cpu]"
)
{
	auto operand = make_sequence_operand<float32_t>({ 2, 3 }, {0, 1, 2, 3, 0, 0});
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 1 };
	check_values<int64_t>(
		xmipp4::count_nonzero(
			operand_ref, make_span(axes), false, context, nullptr
		),
		{ 2 },
		{ 2, 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"any holds when an element is set and all when every one is",
	"[array_reduction][cpu]"
)
{
	check_reduction<any_operation>(
		xmipp4::any,
		{ 2 },
		false,
		element_value(1),
		[](auto, std::size_t) { return true; }
	);
	check_reduction<all_operation>(
		xmipp4::all,
		{ 2 },
		false,
		element_value(1),
		[](auto, std::size_t) { return true; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"any fails and all fails when no element is set",
	"[array_reduction][cpu]"
)
{
	check_reduction<any_operation>(
		xmipp4::any,
		{ 2 },
		false,
		element_value(0),
		[](auto, std::size_t) { return false; }
	);
	check_reduction<all_operation>(
		xmipp4::all,
		{ 2 },
		false,
		element_value(0),
		[](auto, std::size_t) { return false; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"any and all separate on a mixture",
	"[array_reduction][cpu]"
)
{
	// The only case that tells the two apart, both answering the same way
	// when every element agrees.
	auto operand = make_sequence_operand<float32_t>({ 2, 3 }, {0, 1, 2, 3, 4, 5});
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 1 };
	check_values<bool>(
		xmipp4::any(operand_ref, make_span(axes), false, context, nullptr),
		{ 2 },
		{ 1, 1 }
	);
	check_values<bool>(
		xmipp4::all(operand_ref, make_span(axes), false, context, nullptr),
		{ 2 },
		{ 0, 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"any and all answer a reduction over no elements with their identity",
	"[array_reduction][cpu]"
)
{
	// A disjunction of nothing holds of nothing and a conjunction of nothing
	// holds of everything, which is what keeps them associative.
	auto operand = make_sequence_operand<float32_t>({ 0, 3 }, {});
	const const_array_ref operand_ref = operand;
	const std::vector<std::ptrdiff_t> axes = { 0 };

	check_values<bool>(
		xmipp4::any(operand_ref, make_span(axes), false, context, nullptr),
		{ 3 },
		{ 0, 0, 0 }
	);
	check_values<bool>(
		xmipp4::all(operand_ref, make_span(axes), false, context, nullptr),
		{ 3 },
		{ 1, 1, 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"mean answers a reduction over no elements with a not-a-number",
	"[array_reduction][cpu]"
)
{
	// A total of nothing divided by nothing. The result type is inexact by
	// the operation's own rule, so it can hold that answer, and NumPy gives
	// the same one.
	auto operand = make_sequence_operand<float32_t>({ 0, 3 }, {});
	const const_array_ref operand_ref = operand;
	const std::vector<std::ptrdiff_t> axes = { 0 };

	const auto result =
		xmipp4::mean(operand_ref, make_span(axes), false, context, nullptr);

	for (const auto value : read_host<float32_t>(result, 3))
	{
		CHECK( std::isnan(value) );
	}
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax and argmin answer with the first place of a constant array",
	"[array_reduction][cpu]"
)
{
	// Every element being equal, both report the first, which is what pins
	// that a tie keeps the earliest place rather than the latest.
	check_reduction<argmax_operation>(
		xmipp4::argmax,
		{ 1 },
		false,
		element_value(2),
		[](auto, std::size_t) { return 0; }
	);
	check_reduction<argmin_operation>(
		xmipp4::argmin,
		{ 1 },
		false,
		element_value(2),
		[](auto, std::size_t) { return 0; }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax and argmin locate the extremum along an axis",
	"[array_reduction][cpu]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<element_value> values = { 5, 1, 9, 3, 7, 2 };
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	SECTION( "along the axis the elements are strided over" )
	{
		const std::vector<std::ptrdiff_t> axes = { 0 };
		check_values<int64_t>(
			xmipp4::argmax(operand_ref, make_span(axes), false, context,
			               nullptr),
			{ 3 },
			{ 0, 1, 0 }
		);
		check_values<int64_t>(
			xmipp4::argmin(operand_ref, make_span(axes), false, context,
			               nullptr),
			{ 3 },
			{ 1, 0, 1 }
		);
	}

	SECTION( "along the axis the elements are contiguous over" )
	{
		const std::vector<std::ptrdiff_t> axes = { 1 };
		check_values<int64_t>(
			xmipp4::argmax(operand_ref, make_span(axes), false, context,
			               nullptr),
			{ 2 },
			{ 2, 1 }
		);
		check_values<int64_t>(
			xmipp4::argmin(operand_ref, make_span(axes), false, context,
			               nullptr),
			{ 2 },
			{ 1, 2 }
		);
	}
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax counts positions with the last reduced axis running fastest",
	"[array_reduction][cpu]"
)
{
	// Reducing several axes at once leaves the answer to say where in the
	// reduced space the element was, and that space is counted the way an
	// array of the reduced extents is laid out, whatever order the axes
	// would be walked in for locality.
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<element_value> values = { 1, 2, 3, 4, 9, 5 };
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 0, 1 };
	check_values<int64_t>(
		xmipp4::argmax(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 4 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax and argmin locate a not-a-number wherever it sits",
	"[array_reduction][cpu]"
)
{
	// The same rule amax and amin take an extremum by: a not-a-number wins
	// against any number, from either side. NumPy's argmax answers with its
	// place rather than skipping over it, which nanargmax is for.
	const std::vector<std::size_t> extents = { 4 };
	const std::vector<element_value> values = {
		1, not_a_number, 3, 2
	};
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 0 };
	check_values<int64_t>(
		xmipp4::argmax(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 1 }
	);
	check_values<int64_t>(
		xmipp4::argmin(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax and argmin keep the first of several not-a-numbers",
	"[array_reduction][cpu]"
)
{
	// Nothing displaces one already held, so a later one does not take its
	// place, the same way a tie keeps the first place it was seen.
	const std::vector<std::size_t> extents = { 4 };
	const std::vector<element_value> values = {
		5, not_a_number, 7, not_a_number
	};
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	const std::vector<std::ptrdiff_t> axes = { 0 };
	check_values<int64_t>(
		xmipp4::argmax(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 1 }
	);
	check_values<int64_t>(
		xmipp4::argmin(operand_ref, make_span(axes), false, context, nullptr),
		{},
		{ 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax and argmin reject a reduction over no elements",
	"[array_reduction][cpu]"
)
{
	auto operand = make_sequence_operand<float32_t>({ 0, 3 }, {});
	const const_array_ref operand_ref = operand;
	const std::vector<std::ptrdiff_t> axes = { 0 };

	CHECK_THROWS_AS(
		xmipp4::argmax(operand_ref, make_span(axes), false, context, nullptr),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		xmipp4::argmin(operand_ref, make_span(axes), false, context, nullptr),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"argmax counts positions across axes that are not adjacent",
	"[array_reduction][cpu]"
)
{
	// Reducing the first and last axes of a rank three array leaves a
	// reduced space of two axes that were never neighbours, so the index
	// cannot fall out of the layout by accident.
	const std::vector<std::size_t> extents = { 2, 2, 3 };
	std::vector<element_value> values(12, element_value(0.0));
	values[1 * 6 + 1 * 3 + 2] = element_value(9.0);  // (1, 1, 2)
	auto operand = make_sequence_operand<float32_t>(extents, values);
	const const_array_ref operand_ref = operand;

	// The reduced space is (axis 0, axis 2), of extents 2 and 3, so the
	// element at (1, 2) of it counts as 1 * 3 + 2. The other output sees
	// nothing but zeros and answers with the first of them.
	const std::vector<std::ptrdiff_t> axes = { 0, 2 };
	check_values<int64_t>(
		xmipp4::argmax(operand_ref, make_span(axes), false, context, nullptr),
		{ 2 },
		{ 0, 5 }
	);
}
