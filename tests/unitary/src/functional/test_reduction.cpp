// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/reduction.hpp>

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

#include "fixtures/verb_dispatch_fixture.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::verb_dispatch_fixture;
using trompeloeil::_;

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"sum dispatches a sum_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<sum_operation>(sum, sum);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"product dispatches a product_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<product_operation>(product, product);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"amin dispatches a amin_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<amin_operation>(amin, amin);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"amax dispatches a amax_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<amax_operation>(amax, amax);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"all dispatches a all_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<all_operation>(all, all);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"any dispatches a any_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<any_operation>(any, any);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"argmin dispatches a argmin_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<argmin_operation>(argmin, argmin);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"argmax dispatches a argmax_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<argmax_operation>(argmax, argmax);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"count_nonzero dispatches a count_nonzero_operation carrying the "
	"resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<count_nonzero_operation>(count_nonzero, count_nonzero);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"mean dispatches a mean_operation carrying the resolved axes",
	"[array_reduction]"
)
{
	check_reduction_verb<mean_operation>(mean, mean);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"a reduction rejects an axis that names none",
	"[array_reduction]"
)
{
	// The rank is known here, so an axis outside it is caught before an
	// operation is ever built.
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage);
	const const_array_ref x = x_array;

	const std::vector<std::ptrdiff_t> axes = { 7 };
	CHECK_THROWS_AS(
		sum(x, make_span(axes), false, context, nullptr),
		std::out_of_range
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"a reduction keeps the reduced axes when asked to",
	"[array_reduction]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage);
	const const_array_ref x = x_array;

	const std::vector<std::ptrdiff_t> axes = { 0 };

	test::dispatch_record record;
	bool kept = false;
	record.inspect = [&kept] (const operation &op)
	{
		const auto &typed = dynamic_cast<const sum_operation&>(op);
		kept = typed.get_shape_policy().get_keep_dimensions();
	};
	REQUIRE_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	sum(x, make_span(axes), true, context, nullptr);

	CHECK( kept );
}
