// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/ops/policies/cross_product_shape_policy.hpp>
#include <xmipp4/ops/policies/matrix_multiply_shape_policy.hpp>
#include <xmipp4/ops/policies/matrix_vector_shape_policy.hpp>
#include <xmipp4/ops/policies/vector_matrix_shape_policy.hpp>

#include <xmipp4/core/dispatch/operand_names.hpp>
#include <xmipp4/core/dispatch/operation_descriptor.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include <stdexcept>
#include <vector>

using namespace xmipp4;
using namespace xmipp4::ops;

namespace
{

using shape_type = operation_shape_policy::shape_type;

const operation_descriptor& product_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("result");
	static REX_CONST_CONSTEXPR auto inputs =
		make_operand_names("left", "right");
	static const operation_descriptor instance =
		make_operation_descriptor(
			"xmipp4.test", "product", outputs, inputs
		);
	return instance;
}

template <typename Policy>
shape_type deduce_one(
	const Policy &policy,
	const shape_type &left,
	const shape_type &right
)
{
	const std::vector<shape_type> inputs = { left, right };
	std::vector<shape_type> outputs(1);
	policy.deduce(
		product_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0];
}

} // anonymous namespace

TEST_CASE(
	"matrix_multiply_shape_policy should contract the inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_multiply_shape_policy::get();
	CHECK( deduce_one(policy, { 2, 3 }, { 3, 4 }) == shape_type{ 2, 4 } );
}

TEST_CASE(
	"matrix_multiply_shape_policy should broadcast the stacked axes",
	"[linalg_shape_policy]"
)
{
	// Everything before the matrix is a stack of independent matrices, so
	// it broadcasts as an elementwise operation would.
	const auto &policy = matrix_multiply_shape_policy::get();
	CHECK( deduce_one(policy, { 5, 2, 3 }, { 3, 4 }) ==
	       shape_type{ 5, 2, 4 } );
	CHECK( deduce_one(policy, { 1, 2, 3 }, { 5, 3, 4 }) ==
	       shape_type{ 5, 2, 4 } );
}

TEST_CASE(
	"matrix_multiply_shape_policy should promote a vector and then drop "
	"the axis it added",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_multiply_shape_policy::get();

	// A vector on the left is a row, and the row axis goes away again.
	CHECK( deduce_one(policy, { 3 }, { 3, 4 }) == shape_type{ 4 } );

	// A vector on the right is a column.
	CHECK( deduce_one(policy, { 2, 3 }, { 3 }) == shape_type{ 2 } );

	// Both, and nothing is left: a scalar, of rank zero.
	CHECK( deduce_one(policy, { 3 }, { 3 }) == shape_type{} );
}

TEST_CASE(
	"matrix_multiply_shape_policy should reject mismatched inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_multiply_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, { 2, 3 }, { 4, 5 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_multiply_shape_policy should reject a scalar operand",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_multiply_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, shape_type{}, { 3, 4 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_vector_shape_policy should contract the inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_vector_shape_policy::get();
	CHECK( deduce_one(policy, { 2, 3 }, { 3 }) == shape_type{ 2 } );
}

TEST_CASE(
	"matrix_vector_shape_policy should broadcast the stacked axes",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_vector_shape_policy::get();
	CHECK( deduce_one(policy, { 5, 2, 3 }, { 3 }) == shape_type{ 5, 2 } );
	CHECK( deduce_one(policy, { 1, 2, 3 }, { 5, 3 }) == shape_type{ 5, 2 } );
}

TEST_CASE(
	"matrix_vector_shape_policy should reject mismatched inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_vector_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, { 2, 3 }, { 4 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_vector_shape_policy should reject a left operand of rank less "
	"than two",
	"[linalg_shape_policy]"
)
{
	// Unlike matrix_multiply_shape_policy, a vector on the left is not
	// promoted into a row: use vecmat for that.
	const auto &policy = matrix_vector_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, shape_type{ 3 }, { 3 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_vector_shape_policy should reject a scalar right operand",
	"[linalg_shape_policy]"
)
{
	const auto &policy = matrix_vector_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, { 2, 3 }, shape_type{}),
		std::invalid_argument
	);
}

TEST_CASE(
	"vector_matrix_shape_policy should contract the inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = vector_matrix_shape_policy::get();
	CHECK( deduce_one(policy, { 3 }, { 3, 4 }) == shape_type{ 4 } );
}

TEST_CASE(
	"vector_matrix_shape_policy should broadcast the stacked axes",
	"[linalg_shape_policy]"
)
{
	const auto &policy = vector_matrix_shape_policy::get();
	CHECK( deduce_one(policy, { 5, 2, 3 }, { 3, 4 }) == shape_type{ 5, 2, 4 } );
	CHECK( deduce_one(policy, { 1, 3 }, { 5, 2, 3, 4 }) ==
	       shape_type{ 5, 2, 4 } );
}

TEST_CASE(
	"vector_matrix_shape_policy should reject mismatched inner extents",
	"[linalg_shape_policy]"
)
{
	const auto &policy = vector_matrix_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, { 3 }, { 4, 5 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"vector_matrix_shape_policy should reject a right operand of rank less "
	"than two",
	"[linalg_shape_policy]"
)
{
	// Unlike matrix_multiply_shape_policy, a vector on the right is not
	// promoted into a column: use matvec for that.
	const auto &policy = vector_matrix_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, { 3 }, shape_type{ 3 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"vector_matrix_shape_policy should reject a scalar left operand",
	"[linalg_shape_policy]"
)
{
	const auto &policy = vector_matrix_shape_policy::get();
	CHECK_THROWS_AS(
		deduce_one(policy, shape_type{}, { 3, 4 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"cross_product_shape_policy should broadcast and keep the axis",
	"[linalg_shape_policy]"
)
{
	// A stack of vectors crosses with a single one without either being
	// written out, and the axis survives with its three components.
	const cross_product_shape_policy policy(1);
	CHECK( deduce_one(policy, { 4, 3 }, { 1, 3 }) == shape_type{ 4, 3 } );
}

TEST_CASE(
	"cross_product_shape_policy should reject an axis that is not three "
	"components long",
	"[linalg_shape_policy]"
)
{
	// Unlike the extents a matrix multiplication contracts, this one is
	// not a free parameter of the problem.
	const cross_product_shape_policy policy(1);
	CHECK_THROWS_AS(
		deduce_one(policy, { 4, 2 }, { 4, 2 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"cross_product_shape_policy should reject an axis beyond the rank",
	"[linalg_shape_policy]"
)
{
	const cross_product_shape_policy policy(5);
	CHECK_THROWS_AS(
		deduce_one(policy, { 4, 3 }, { 4, 3 }),
		std::out_of_range
	);
}
