// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/ops/rules/operand_shape_rules.hpp>

#include "../../core/dispatch/rules/shape_rule_calls.hpp"

#include <cstddef>
#include <stdexcept>

using namespace rexlib;
using namespace rexlib::ops;

using test::accept_shapes;
using test::check_shape_instance;
using test::deduce_shape;
using test::deduce_shapes;
using test::shape_type;

TEST_CASE(
	"elementwise shape rules should give every output the shape the inputs "
	"broadcast to",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<unary_elementwise_shape_rule>({ { 3, 5 } }) ==
		shape_type{ 3, 5 }
	);
	CHECK(
		deduce_shape<binary_elementwise_shape_rule>({ { 1, 8 }, { 4, 1 } }) ==
		shape_type{ 4, 8 }
	);
	CHECK(
		deduce_shape<ternary_elementwise_shape_rule>(
			{ { 4, 1, 1 }, { 3, 1 }, { 2 } }
		) == shape_type{ 4, 3, 2 }
	);

	const auto pair = deduce_shapes<unary_pair_elementwise_shape_rule>(
		2,
		{ { 3 } }
	);
	CHECK( pair[0] == shape_type{ 3 } );
	CHECK( pair[1] == shape_type{ 3 } );

	const auto binary_pair =
		deduce_shapes<binary_pair_elementwise_shape_rule>(
			2,
			{ { 1, 8 }, { 4, 1 } }
		);
	CHECK( binary_pair[0] == shape_type{ 4, 8 } );
	CHECK( binary_pair[1] == shape_type{ 4, 8 } );
}

TEST_CASE(
	"elementwise shape rules should reject inputs that do not broadcast",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<binary_elementwise_shape_rule>({ { 4, 2 }, { 3 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"nullary_elementwise_shape_rule should write into any output it is "
	"handed",
	"[operand_shape_rules]"
)
{
	CHECK( deduce_shape<nullary_elementwise_shape_rule>({}).empty() );
	CHECK_NOTHROW(
		accept_shapes<nullary_elementwise_shape_rule>({ { 4, 8 } }, {})
	);
}

TEST_CASE(
	"elementwise shape rules should accept outputs the inputs broadcast to",
	"[operand_shape_rules]"
)
{
	CHECK_NOTHROW(
		accept_shapes<binary_pair_elementwise_shape_rule>(
			{ { 4, 8 }, { 4, 8 } },
			{ { 1, 8 }, { 1, 8 } }
		)
	);
}

TEST_CASE(
	"elementwise shape rules should reject outputs of different shapes",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		accept_shapes<binary_pair_elementwise_shape_rule>(
			{ { 4, 8 }, { 4, 9 } },
			{ { 4, 8 }, { 4, 8 } }
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"elementwise shape rules should reject an output the inputs do not "
	"broadcast to",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		accept_shapes<unary_elementwise_shape_rule>({ { 3, 8 } }, { { 4, 8 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"reduction_shape_rule should drop or keep the reduced axes",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 4, 8 } }, { 1 }) ==
		shape_type{ 4 }
	);
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 4, 8 } }, { 1 }, true) ==
		shape_type{ 4, 1 }
	);
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 2, 3, 4 } }, { 0, 2 }) ==
		shape_type{ 3 }
	);
}

TEST_CASE(
	"reduction_shape_rule should reduce over every axis, or none",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 2, 3 } }, { 0, 1 }) ==
		shape_type{}
	);
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 2, 3 } }) ==
		shape_type{ 2, 3 }
	);
}

TEST_CASE(
	"reduction_shape_rule should reduce the axes in whatever order they are "
	"named",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 2, 3, 4 } }, { 2, 0 }) ==
		shape_type{ 3 }
	);
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 2, 3, 4 } }, { 2, 0 }, true) ==
		shape_type{ 1, 3, 1 }
	);
}

TEST_CASE(
	"reduction_shape_rule should reject a repeated axis or one beyond the "
	"rank",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		check_shape_instance<reduction_shape_rule>({ 1, 1 }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<reduction_shape_rule>({ { 2, 3 } }, { 2 }),
		std::out_of_range
	);
}

TEST_CASE(
	"reduction_shape_rule should admit reducing an empty axis",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<reduction_shape_rule>({ { 0, 3 } }, { 0 }) ==
		shape_type{ 3 }
	);
}

TEST_CASE(
	"extremum_reduction_shape_rule should reject reducing an empty axis",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<extremum_reduction_shape_rule>({ { 0, 3 } }, { 0 }),
		std::invalid_argument
	);
	CHECK(
		deduce_shape<extremum_reduction_shape_rule>({ { 0, 3 } }, { 1 }) ==
		shape_type{ 0 }
	);
}

TEST_CASE(
	"vector_dot_shape_rule should contract the named axis",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<vector_dot_shape_rule>({ { 4, 3 }, { 3 } }, { 1 }) ==
		shape_type{ 4 }
	);
	CHECK(
		deduce_shape<vector_dot_shape_rule>(
			{ { 4, 3 }, { 3 } },
			{ 1 },
			true
		) == shape_type{ 4, 1 }
	);
}

TEST_CASE(
	"vector_dot_shape_rule should not broadcast the contracted axis",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<vector_dot_shape_rule>({ { 4, 3 }, { 4, 1 } }, { 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"cross_product_shape_rule should broadcast the stacks and keep the axis",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<cross_product_shape_rule>({ { 4, 3 }, { 1, 3 } }, { 1 }) ==
		shape_type{ 4, 3 }
	);
}

TEST_CASE(
	"cross_product_shape_rule should require three components on both "
	"operands",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<cross_product_shape_rule>({ { 4, 2 }, { 4, 2 } }, { 1 }),
		std::invalid_argument
	);

	// A single component does not broadcast against three.
	CHECK_THROWS_AS(
		deduce_shape<cross_product_shape_rule>({ { 4, 3 }, { 4, 1 } }, { 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"cross_product_shape_rule should reject an axis beyond the rank",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<cross_product_shape_rule>({ { 4, 3 }, { 4, 3 } }, { 5 }),
		std::out_of_range
	);
}

TEST_CASE(
	"axiswise_shape_rule should leave the shape alone",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<axiswise_shape_rule>({ { 4, 8, 16 } }, { 1, 2 }) ==
		shape_type{ 4, 8, 16 }
	);
	CHECK(
		deduce_shape<axiswise_shape_rule>({ { 4, 0 } }, { 1 }) ==
		shape_type{ 4, 0 }
	);
	CHECK_THROWS_AS(
		deduce_shape<axiswise_shape_rule>({ { 4, 8 } }, { 5 }),
		std::out_of_range
	);
}

TEST_CASE(
	"fourier_transform_shape_rule should leave the shape alone and reject "
	"an empty transformed axis",
	"[operand_shape_rules]"
)
{
	CHECK(
		deduce_shape<fourier_transform_shape_rule>({ { 4, 8 } }, { 1 }) ==
		shape_type{ 4, 8 }
	);
	CHECK(
		deduce_shape<fourier_transform_shape_rule>({ { 0, 8 } }, { 1 }) ==
		shape_type{ 0, 8 }
	);
	CHECK_THROWS_AS(
		deduce_shape<fourier_transform_shape_rule>({ { 4, 0 } }, { 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"real_fourier_transform_shape_rule should halve only the last axis "
	"named, rounding down",
	"[operand_shape_rules]"
)
{
	using rule = real_fourier_transform_shape_rule;
	CHECK(
		deduce_shape<rule>({ { 4, 8, 16 } }, { 1, 2 }) ==
		shape_type{ 4, 8, 9 }
	);
	CHECK( deduce_shape<rule>({ { 7 } }, { 0 }) == shape_type{ 4 } );
	CHECK( deduce_shape<rule>({ { 8 } }, { 0 }) == shape_type{ 5 } );
	CHECK(
		deduce_shape<rule>({ { 8, 4, 16 } }, { 2, 0 }) ==
		shape_type{ 5, 4, 16 }
	);
}

TEST_CASE(
	"real_fourier_transform_shape_rule should reject a repeated axis, an "
	"axis beyond the rank and no axis at all",
	"[operand_shape_rules]"
)
{
	using rule = real_fourier_transform_shape_rule;
	CHECK_THROWS_AS(
		check_shape_instance<rule>({ 1, 0, 1 }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 4, 8 } }, { 3, 0 }),
		std::out_of_range
	);
	CHECK_THROWS_AS(
		check_shape_instance<rule>({}),
		std::invalid_argument
	);
}

TEST_CASE(
	"real_fourier_transform_shape_rule should reject an empty transformed "
	"axis",
	"[operand_shape_rules]"
)
{
	using rule = real_fourier_transform_shape_rule;
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 0 } }, { 0 }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 0, 4 } }, { 0, 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"inverse_real_fourier_transform_shape_rule should restore the last axis "
	"named to the extent the operation fixes",
	"[operand_shape_rules]"
)
{
	using rule = inverse_real_fourier_transform_shape_rule;
	CHECK(
		deduce_shape<rule>({ { 4, 9 } }, { 1 }, false, { 16 }) ==
		shape_type{ 4, 16 }
	);
	CHECK(
		deduce_shape<rule>({ { 4, 9 } }, { 1 }, false, { 17 }) ==
		shape_type{ 4, 17 }
	);
	CHECK(
		deduce_shape<rule>({ { 8, 9 } }, { 0, 1 }, false, { 16 }) ==
		shape_type{ 8, 16 }
	);
	CHECK(
		deduce_shape<rule>({ { 5, 6 } }, { 1, 0 }, false, { 8 }) ==
		shape_type{ 8, 6 }
	);
}

TEST_CASE(
	"inverse_real_fourier_transform_shape_rule should reject a spectrum "
	"that does not hold the coefficients of the extent",
	"[operand_shape_rules]"
)
{
	using rule = inverse_real_fourier_transform_shape_rule;
	const auto stored = GENERATE(
		std::size_t(0),
		std::size_t(5),
		std::size_t(7)
	);

	// A signal of 10 samples has 6 coefficients.
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { stored } }, { 0 }, false, { 10 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"inverse_real_fourier_transform_shape_rule should reject restoring an "
	"axis with no samples",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		check_shape_instance<inverse_real_fourier_transform_shape_rule>(
			{ 0 },
			{ 0 }
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"the real transform and its inverse should agree on every extent",
	"[operand_shape_rules]"
)
{
	for (std::size_t extent = 1; extent <= 32; ++extent)
	{
		const auto stored = deduce_shape<real_fourier_transform_shape_rule>(
			{ { extent } },
			{ 0 }
		);

		INFO( "extent " << extent );
		CHECK(
			deduce_shape<inverse_real_fourier_transform_shape_rule>(
				{ stored },
				{ 0 },
				false,
				{ extent }
			) == shape_type{ extent }
		);
	}
}

TEST_CASE(
	"matrix_multiply_shape_rule should contract the inner extents and "
	"broadcast the stacks",
	"[operand_shape_rules]"
)
{
	using rule = matrix_multiply_shape_rule;
	CHECK( deduce_shape<rule>({ { 2, 3 }, { 3, 4 } }) == shape_type{ 2, 4 } );
	CHECK(
		deduce_shape<rule>({ { 5, 2, 3 }, { 3, 4 } }) ==
		shape_type{ 5, 2, 4 }
	);
	CHECK(
		deduce_shape<rule>({ { 1, 2, 3 }, { 5, 3, 4 } }) ==
		shape_type{ 5, 2, 4 }
	);
}

TEST_CASE(
	"matrix_multiply_shape_rule should reject mismatched inner extents",
	"[operand_shape_rules]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<matrix_multiply_shape_rule>({ { 2, 3 }, { 4, 5 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_multiply_shape_rule should reject an operand that is not a "
	"matrix",
	"[operand_shape_rules]"
)
{
	// No backend multiplies a vector as a matrix, so none is promoted.
	using rule = matrix_multiply_shape_rule;
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 3 }, { 3, 4 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 2, 3 }, { 3 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ {}, { 3, 4 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"matrix_vector_shape_rule should contract the inner extents and "
	"broadcast the stacks",
	"[operand_shape_rules]"
)
{
	using rule = matrix_vector_shape_rule;
	CHECK( deduce_shape<rule>({ { 2, 3 }, { 3 } }) == shape_type{ 2 } );
	CHECK( deduce_shape<rule>({ { 5, 2, 3 }, { 3 } }) == shape_type{ 5, 2 } );
	CHECK(
		deduce_shape<rule>({ { 1, 2, 3 }, { 5, 3 } }) ==
		shape_type{ 5, 2 }
	);
}

TEST_CASE(
	"matrix_vector_shape_rule should reject mismatched inner extents and "
	"operands of too few axes",
	"[operand_shape_rules]"
)
{
	using rule = matrix_vector_shape_rule;
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 2, 3 }, { 4 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 3 }, { 3 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 2, 3 }, {} }),
		std::invalid_argument
	);
}

TEST_CASE(
	"vector_matrix_shape_rule should contract the inner extents and "
	"broadcast the stacks",
	"[operand_shape_rules]"
)
{
	using rule = vector_matrix_shape_rule;
	CHECK( deduce_shape<rule>({ { 3 }, { 3, 4 } }) == shape_type{ 4 } );
	CHECK(
		deduce_shape<rule>({ { 5, 2, 3 }, { 3, 4 } }) ==
		shape_type{ 5, 2, 4 }
	);
	CHECK(
		deduce_shape<rule>({ { 1, 3 }, { 5, 2, 3, 4 } }) ==
		shape_type{ 5, 2, 4 }
	);
}

TEST_CASE(
	"vector_matrix_shape_rule should reject mismatched inner extents and "
	"operands of too few axes",
	"[operand_shape_rules]"
)
{
	using rule = vector_matrix_shape_rule;
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 4 }, { 3, 4 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 3 }, { 3 } }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		deduce_shape<rule>({ {}, { 3, 4 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"sequence_shape_rule should give a sequence of the length the operation "
	"names",
	"[operand_shape_rules]"
)
{
	using rule = sequence_shape_rule;
	CHECK( deduce_shape<rule>({}, {}, false, { 5 }) == shape_type{ 5 } );
	CHECK( deduce_shape<rule>({}, {}, false, { 0 }) == shape_type{ 0 } );
}

TEST_CASE(
	"sequence_shape_rule should reject an input and require an output of "
	"exactly its length",
	"[operand_shape_rules]"
)
{
	using rule = sequence_shape_rule;
	CHECK_THROWS_AS(
		deduce_shape<rule>({ { 5 } }, {}, false, { 5 }),
		std::invalid_argument
	);
	CHECK_NOTHROW( accept_shapes<rule>({ { 5 } }, {}, {}, false, { 5 }) );
	CHECK_THROWS_AS(
		accept_shapes<rule>({ { 6 } }, {}, {}, false, { 5 }),
		std::invalid_argument
	);
	CHECK_THROWS_AS(
		accept_shapes<rule>({ { 2, 5 } }, {}, {}, false, { 5 }),
		std::invalid_argument
	);
}
