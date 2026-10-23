// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/operation_pivots.hpp>

#include <rexlib/core/dispatch/basic_operation.hpp>
#include <rexlib/core/dispatch/parametric_operation.hpp>
#include <rexlib/core/dispatch/rules/operand_type_rule.hpp>
#include <rexlib/ops/ops_component.hpp>
#include <rexlib/ops/rules/operand_shape_rules.hpp>
#include <rexlib/ops/rules/operand_type_rules.hpp>

using namespace rexlib;

namespace
{

class target_type
{
public:
	explicit target_type(numerical_type type) noexcept
		: m_type(type)
	{
	}

	numerical_type get_type() const noexcept
	{
		return m_type;
	}

private:
	numerical_type m_type;
};

struct type_of
{
	template <typename Parameters>
	static numerical_type get(const Parameters &parameters) noexcept
	{
		return parameters.get_type();
	}
};

REXLIB_DECLARE_PARAMETRIC_OPERATION(
	pivot_astype,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::unary_elementwise_shape_rule,
	target_type,
	operand_type_rule<
		type_list<pivot_from_operation<type_of>, pivot_from_input<0>>,
		type_list<slot_same_as<0>>,
		type_list<slot_same_as<1>>
	>
);

REXLIB_DECLARE_OPERATION(
	pivot_negate,
	ops::ops_component,
	REXLIB_OPERANDS("result"),
	REXLIB_OPERANDS("value"),
	ops::unary_elementwise_shape_rule,
	ops::unary_homogeneous_rule<>
);

} // anonymous namespace

TEST_CASE(
	"get_operation_pivots should read the data types an operation supplies",
	"[operation_pivots]"
)
{
	const pivot_astype_operation operation(numerical_type::float64);
	const auto pivots = get_operation_pivots(operation);
	REQUIRE( pivots.size() == 1 );
	CHECK( pivots[0] == numerical_type::float64 );
}

TEST_CASE(
	"get_operation_pivots should read nothing from an operation whose rule "
	"has no operation supplied pivot",
	"[operation_pivots]"
)
{
	const pivot_negate_operation operation;
	CHECK( get_operation_pivots(operation).empty() );
}
