// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/operation/type_rule/type_rule_accessors.hpp>

#include <rexlib/core/operation/type_rule/operand_type_rule.hpp>

using namespace rexlib;

namespace
{

class test_parameters
{
public:
	numerical_type get_first() const noexcept
	{
		return numerical_type::float32;
	}

	numerical_type get_second() const noexcept
	{
		return numerical_type::int8;
	}
};

struct test_first
{
	template <typename Parameters>
	static numerical_type get(const Parameters &parameters) noexcept
	{
		return parameters.get_first();
	}
};

struct test_second
{
	template <typename Parameters>
	static numerical_type get(const Parameters &parameters) noexcept
	{
		return parameters.get_second();
	}
};

using pair_rule = operand_type_rule<
	type_list<
		pivot_from_operation<test_first>,
		pivot_from_input<0>,
		pivot_from_operation<test_second>
	>,
	type_list<slot_same_as<0>, slot_same_as<2>>,
	type_list<slot_same_as<1>>
>;

using input_rule = operand_type_rule<
	type_list<pivot_from_input<0>>,
	type_list<slot_same_as<0>>,
	type_list<slot_same_as<0>>
>;

} // anonymous namespace

TEST_CASE(
	"type_rule_accessors should read the data types the operation supplies "
	"in declaration order",
	"[type_rule_accessors]"
)
{
	STATIC_CHECK( type_rule_accessors<pair_rule>::operation_pivot_count == 2 );

	const auto pivots =
		type_rule_accessors<pair_rule>::get_operation_data_types(test_parameters());
	CHECK( pivots[0] == numerical_type::float32 );
	CHECK( pivots[1] == numerical_type::int8 );
}

TEST_CASE(
	"type_rule_accessors should read nothing for a rule fixed by its inputs",
	"[type_rule_accessors]"
)
{
	STATIC_CHECK( type_rule_accessors<input_rule>::operation_pivot_count == 0 );
	CHECK(
		type_rule_accessors<input_rule>::get_operation_data_types(
			test_parameters()
		).empty()
	);
}
