// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/core/operation/shape_rule/shape_rule_interpreter.hpp>

#include "fixtures/doubled_extent_transform.hpp"
#include "fixtures/shape_rule_calls.hpp"

#include <rexlib/core/operation/shape_rule/named_core.hpp>
#include <rexlib/core/operation/shape_rule/operand_shape_rule.hpp>
#include <rexlib/core/operation/shape_rule/positive_extent_domain.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using Catch::Matchers::ContainsSubstring;

namespace
{

using test::accept_shapes;
using test::check_shape_instance;
using test::deduce_shape;
using test::deduce_shapes;
using test::extent_list;
using test::make_shape_rule_interpreter;
using test::make_shaped_operands;
using test::shape_rule_test_descriptor;
using test::shape_type;

struct test_axes
{
};

struct test_keep
{
};

struct test_extent
{
};

using binary_elementwise_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>, core_dimensions<>>,
	type_list<core_dimensions<>, core_dimensions<>>,
	trailing_core,
	output_batch::broadcast_target
>;

using nullary_elementwise_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<>,
	trailing_core,
	output_batch::broadcast_target
>;

using matrix_product_rule = operand_shape_rule<
	type_list<extent_from_inputs<>, extent_from_inputs<>, extent_from_inputs<>>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<2>>>,
	type_list<
		core_dimensions<axis_same_as<0>, axis_same_as<1>>,
		core_dimensions<axis_same_as<1>, axis_same_as<2>>
	>
>;

using cross_product_rule = operand_shape_rule<
	type_list<extent_fixed<3>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>>
	>,
	named_core<test_axes>
>;

using fixed_output_rule = operand_shape_rule<
	type_list<extent_fixed<3>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<core_dimensions<>>
>;

using doubled_fixed_rule = operand_shape_rule<
	type_list<extent_fixed<3>>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<test::axis_doubled_of<0>>>
>;

using supplied_extent_rule = operand_shape_rule<
	type_list<extent_from_operation<test_extent>>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_same_as<0>>>
>;

using reduction_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<test_axes, test_keep>
>;

using extremum_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<positive_extent_domain>>>,
	named_core<test_axes, test_keep>
>;

using vector_dot_rule = operand_shape_rule<
	type_list<extent_from_inputs<>>,
	type_list<core_dimensions<>>,
	type_list<
		core_dimensions<axis_same_as<0>>,
		core_dimensions<axis_same_as<0>>
	>,
	named_core<test_axes>
>;

using doubled_core_rule = operand_shape_rule<
	type_list<extent_from_inputs<positive_extent_domain>>,
	type_list<core_dimensions<axis_run<>, test::axis_doubled_of<0>>>,
	type_list<core_dimensions<axis_run<>, axis_same_as<0>>>,
	named_core<test_axes>
>;

using doubled_input_rule = operand_shape_rule<
	type_list<extent_from_operation<test_extent, positive_extent_domain>>,
	type_list<core_dimensions<axis_run<>, axis_same_as<0>>>,
	type_list<core_dimensions<axis_run<>, test::axis_doubled_of<0>>>,
	named_core<test_axes>
>;

using sequence_rule = operand_shape_rule<
	type_list<extent_from_operation<test_extent>>,
	type_list<core_dimensions<axis_same_as<0>>>,
	type_list<>
>;

} // anonymous namespace

TEST_CASE(
	"shape_rule_interpreter should broadcast the inputs of an operation "
	"without core axes",
	"[shape_rule_interpreter]"
)
{
	const auto shapes =
		deduce_shapes<binary_elementwise_rule>(2, { { 4, 1 }, { 3 } });
	CHECK( shapes[0] == shape_type{ 4, 3 } );
	CHECK( shapes[1] == shape_type{ 4, 3 } );
}

TEST_CASE(
	"shape_rule_interpreter should name the inputs that do not broadcast",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_AS(
		deduce_shapes<binary_elementwise_rule>(2, { { 4, 2 }, { 3 } }),
		std::invalid_argument
	);
	CHECK_THROWS_WITH(
		deduce_shapes<binary_elementwise_rule>(2, { { 4, 2 }, { 3 } }),
		ContainsSubstring("'left' and 'right' do not broadcast")
	);
}

TEST_CASE(
	"shape_rule_interpreter should give an operation without inputs no axes",
	"[shape_rule_interpreter]"
)
{
	CHECK( deduce_shape<nullary_elementwise_rule>({}) == shape_type{} );
}

TEST_CASE(
	"shape_rule_interpreter should let an operation without core axes write "
	"into outputs its inputs broadcast to",
	"[shape_rule_interpreter]"
)
{
	CHECK_NOTHROW(
		accept_shapes<binary_elementwise_rule>(
			{ { 2, 4, 3 }, { 2, 4, 3 } },
			{ { 4, 1 }, { 3 } }
		)
	);
	CHECK_THROWS_AS(
		accept_shapes<binary_elementwise_rule>(
			{ { 4, 2 }, { 4, 2 } },
			{ { 4, 1 }, { 3 } }
		),
		std::invalid_argument
	);
	CHECK_THROWS_WITH(
		accept_shapes<binary_elementwise_rule>(
			{ { 4, 3 }, { 2, 4, 3 } },
			{ { 4, 1 }, { 3 } }
		),
		ContainsSubstring("'other_result' does not have the shape of")
	);
}

TEST_CASE(
	"shape_rule_interpreter should give an operation without inputs the "
	"outputs it is handed",
	"[shape_rule_interpreter]"
)
{
	CHECK_NOTHROW( accept_shapes<nullary_elementwise_rule>({ { 5, 7 } }, {}) );
}

TEST_CASE(
	"shape_rule_interpreter should multiply stacks of matrices",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<matrix_product_rule>({ { 5, 3 }, { 3, 4 } }) ==
		shape_type{ 5, 4 }
	);
	CHECK(
		deduce_shape<matrix_product_rule>({ { 2, 1, 5, 3 }, { 7, 3, 4 } }) ==
		shape_type{ 2, 7, 5, 4 }
	);
}

TEST_CASE(
	"shape_rule_interpreter should require core axes sharing an extent "
	"variable to agree",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		deduce_shape<matrix_product_rule>({ { 5, 3 }, { 4, 4 } }),
		ContainsSubstring("'right' has extent 4 along axis 0") &&
		ContainsSubstring("the extent of axis 1 of input operand 'left'")
	);

	// A core axis of one does not broadcast against the other.
	CHECK_THROWS_AS(
		deduce_shape<matrix_product_rule>({ { 5, 3 }, { 1, 4 } }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject an input with fewer axes than its "
	"core",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		deduce_shape<matrix_product_rule>({ { 3 }, { 3, 4 } }),
		ContainsSubstring("'left' has 1 axis, but this operation needs at "
			"least 2")
	);
}

TEST_CASE(
	"shape_rule_interpreter should accept only the outputs the inputs call "
	"for when output batches are exact",
	"[shape_rule_interpreter]"
)
{
	CHECK_NOTHROW(
		accept_shapes<matrix_product_rule>(
			{ { 2, 5, 4 } },
			{ { 2, 5, 3 }, { 3, 4 } }
		)
	);
	CHECK_THROWS_WITH(
		accept_shapes<matrix_product_rule>(
			{ { 1, 5, 4 } },
			{ { 2, 5, 3 }, { 3, 4 } }
		),
		ContainsSubstring("has shape (1, 5, 4), but the inputs call for "
			"(2, 5, 4)")
	);
}

TEST_CASE(
	"shape_rule_interpreter should place named core axes in the frame the "
	"inputs broadcast to",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<cross_product_rule>({ { 4, 3 }, { 3 } }, { 1 }) ==
		shape_type{ 4, 3 }
	);
}

TEST_CASE(
	"shape_rule_interpreter should require a fixed extent along a fixed "
	"core axis",
	"[shape_rule_interpreter]"
)
{
	// Broadcasting the core axis is not allowed either.
	CHECK_THROWS_WITH(
		deduce_shape<cross_product_rule>({ { 4, 3 }, { 4, 1 } }, { 1 }),
		ContainsSubstring("'right' has extent 1 along axis 1, but this "
			"operation requires 3.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject a named axis beyond the frame",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<cross_product_rule>({ { 4, 3 }, { 3 } }, { 2 }),
		std::out_of_range
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject an input lacking a named axis",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		deduce_shape<cross_product_rule>({ { 3, 4 }, { 4 } }, { 0 }),
		ContainsSubstring("'right' has no axis 0")
	);
}

TEST_CASE(
	"shape_rule_interpreter should require the number of named axes the "
	"core declares",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		check_shape_instance<cross_product_rule>({ 0, 1 }),
		ContainsSubstring("2 axes are named, but this operation acts along "
			"exactly 1 axis.")
	);
	CHECK_THROWS_AS(
		deduce_shape<cross_product_rule>({ { 3, 3 }, { 3, 3 } }, { 0, 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject an axis named twice",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		check_shape_instance<reduction_rule>({ 1, 0, 1 }),
		ContainsSubstring("axis 1 is named more than once")
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject axes named for a rule whose core "
	"lies last",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_AS(
		check_shape_instance<matrix_product_rule>({ 0 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should drop or keep the axes an output without "
	"core axes leaves out",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<reduction_rule>({ { 4, 5, 6 } }, { 0, 2 }) ==
		shape_type{ 5 }
	);
	CHECK(
		deduce_shape<reduction_rule>({ { 4, 5, 6 } }, { 2, 0 }, true) ==
		shape_type{ 1, 5, 1 }
	);
	CHECK(
		deduce_shape<reduction_rule>({ { 4, 5, 6 } }) ==
		shape_type{ 4, 5, 6 }
	);
}

TEST_CASE(
	"shape_rule_interpreter should check the axes of a run against its "
	"domain",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<extremum_rule>({ { 4, 2 } }, { 1 }) == shape_type{ 4 }
	);
	CHECK_THROWS_WITH(
		deduce_shape<extremum_rule>({ { 4, 0 } }, { 1 }),
		ContainsSubstring("'left' has extent 0 along axis 1, but this "
			"operation requires at least one.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should contract a named axis without "
	"broadcasting it",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<vector_dot_rule>({ { 4, 3 }, { 3 } }, { 1 }) ==
		shape_type{ 4 }
	);
	CHECK_THROWS_AS(
		deduce_shape<vector_dot_rule>({ { 4, 3 }, { 4, 1 } }, { 1 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should transform the last axis named",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<doubled_core_rule>({ { 8, 6 } }, { 0, 1 }) ==
		shape_type{ 8, 12 }
	);
	CHECK(
		deduce_shape<doubled_core_rule>({ { 8, 6 } }, { 1, 0 }) ==
		shape_type{ 16, 6 }
	);
}

TEST_CASE(
	"shape_rule_interpreter should check an extent variable bound by an "
	"input against its domain",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_AS(
		deduce_shape<doubled_core_rule>({ { 0 } }, { 0 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should require a run the core declares beside "
	"other axes to leave room for them",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		check_shape_instance<doubled_core_rule>({}),
		ContainsSubstring("acts along at least 1 axis.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should take an extent the operation fixes",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<doubled_input_rule>(
			{ { 4, 20 } },
			{ 1 },
			false,
			{ 10 }
		) == shape_type{ 4, 10 }
	);
	CHECK_THROWS_WITH(
		deduce_shape<doubled_input_rule>(
			{ { 4, 5 } },
			{ 1 },
			false,
			{ 10 }
		),
		ContainsSubstring("'left' has extent 5 along axis 1, but this "
			"operation requires 20, twice the extent of 10, which "
			"the operation fixes.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should check the extents the operation fixes",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		check_shape_instance<doubled_input_rule>({ 0 }, { 0 }),
		ContainsSubstring("fixes an extent of 0, but this operation "
			"requires at least one.")
	);
	CHECK_THROWS_WITH(
		check_shape_instance<doubled_input_rule>({ 0 }),
		ContainsSubstring("supplies 0 extents, but its shape rule reads 1.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should write a sequence of the extent the "
	"operation fixes",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<sequence_rule>({}, {}, false, { 5 }) == shape_type{ 5 }
	);
	CHECK_NOTHROW(
		accept_shapes<sequence_rule>({ { 5 } }, {}, {}, false, { 5 })
	);
	CHECK_THROWS_AS(
		accept_shapes<sequence_rule>({ { 2, 5 } }, {}, {}, false, { 5 }),
		std::invalid_argument
	);
}

TEST_CASE(
	"shape_rule_interpreter should reject a number of operands its rule does "
	"not describe",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		deduce_shape<matrix_product_rule>({ { 5, 3 } }),
		ContainsSubstring("the shape rule describes 1 output and 2 inputs, "
			"but 1 output and 1 input were given.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should leave data types alone",
	"[shape_rule_interpreter]"
)
{
	const auto inputs = make_shaped_operands({ { 4 }, { 4 } });
	std::vector<operand_specification> outputs = {
		operand_specification(shape_type{}, numerical_type::int8),
		operand_specification(shape_type{}, numerical_type::unknown)
	};
	make_shape_rule_interpreter<binary_elementwise_rule>().deduce(
		shape_rule_test_descriptor(),
		span<const std::size_t>(),
		false,
		span<const std::size_t>(),
		make_span(outputs),
		make_span(inputs)
	);
	CHECK( outputs[0].get_data_type() == numerical_type::int8 );
	CHECK( outputs[1].get_data_type() == numerical_type::unknown );
}

TEST_CASE(
	"shape_rule_interpreter should give an output an extent its rule fixes",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<fixed_output_rule>({ { 4 } }) == shape_type{ 4, 3 }
	);
}

TEST_CASE(
	"shape_rule_interpreter should name the relation to a fixed extent it "
	"transforms",
	"[shape_rule_interpreter]"
)
{
	CHECK(
		deduce_shape<doubled_fixed_rule>({ { 4, 6 } }) == shape_type{ 4 }
	);
	CHECK_THROWS_WITH(
		deduce_shape<doubled_fixed_rule>({ { 4, 5 } }),
		ContainsSubstring("'left' has extent 5 along axis 1, but this "
			"operation requires 6, twice the extent of 3.")
	);
}

TEST_CASE(
	"shape_rule_interpreter should not restate an extent the operation "
	"supplies as it is",
	"[shape_rule_interpreter]"
)
{
	CHECK_THROWS_WITH(
		deduce_shape<supplied_extent_rule>({ { 4, 5 } }, {}, false, { 6 }),
		ContainsSubstring("'left' has extent 5 along axis 1, but this "
			"operation requires 6, which the operation fixes.")
	);
}
