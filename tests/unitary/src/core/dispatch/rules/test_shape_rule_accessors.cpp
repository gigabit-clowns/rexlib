// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/dispatch/rules/shape_rule_accessors.hpp>

#include <rexlib/core/dispatch/rules/core_dimensions.hpp>
#include <rexlib/core/dispatch/rules/operand_shape_rule.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;

namespace
{

class test_parameters
{
public:
	span<const std::size_t> get_axes() const noexcept
	{
		return make_span(m_axes.data(), m_axes.size());
	}

	bool get_keep() const noexcept
	{
		return true;
	}

	std::size_t get_first() const noexcept
	{
		return 5;
	}

	std::size_t get_second() const noexcept
	{
		return 7;
	}

private:
	std::vector<std::size_t> m_axes = { 2, 0 };
};

struct test_axes
{
	template <typename Parameters>
	static span<const std::size_t> get(const Parameters &parameters) noexcept
	{
		return parameters.get_axes();
	}
};

struct test_keep
{
	template <typename Parameters>
	static bool get(const Parameters &parameters) noexcept
	{
		return parameters.get_keep();
	}
};

struct test_first
{
	template <typename Parameters>
	static std::size_t get(const Parameters &parameters) noexcept
	{
		return parameters.get_first();
	}
};

struct test_second
{
	template <typename Parameters>
	static std::size_t get(const Parameters &parameters) noexcept
	{
		return parameters.get_second();
	}
};

using named_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<test_axes, test_keep>
>;

using named_dropping_rule = operand_shape_rule<
	type_list<>,
	type_list<core_dimensions<>>,
	type_list<core_dimensions<axis_run<>>>,
	named_core<test_axes>
>;

using extent_rule = operand_shape_rule<
	type_list<
		extent_from_operation<test_first>,
		extent_from_inputs<>,
		extent_from_operation<test_second>
	>,
	type_list<core_dimensions<axis_same_as<0>, axis_same_as<2>>>,
	type_list<core_dimensions<axis_same_as<1>>>
>;

} // anonymous namespace

TEST_CASE(
	"shape_rule_accessors should read the named axes and whether they are "
	"kept",
	"[shape_rule_accessors]"
)
{
	const test_parameters parameters;
	const auto axes =
		shape_rule_accessors<named_rule>::get_named_axes(parameters);
	CHECK( axes.data() == parameters.get_axes().data() );
	CHECK( shape_rule_accessors<named_rule>::get_keep_dimensions(parameters) );
}

TEST_CASE(
	"shape_rule_accessors should report no kept axes when the rule names no "
	"keep accessor",
	"[shape_rule_accessors]"
)
{
	const test_parameters parameters;
	CHECK_FALSE(
		shape_rule_accessors<named_dropping_rule>::get_keep_dimensions(
			parameters
		)
	);
}

TEST_CASE(
	"shape_rule_accessors should report no named axes under trailing "
	"placement",
	"[shape_rule_accessors]"
)
{
	const test_parameters parameters;
	CHECK(
		shape_rule_accessors<extent_rule>::get_named_axes(parameters).empty()
	);
	CHECK_FALSE(
		shape_rule_accessors<extent_rule>::get_keep_dimensions(parameters)
	);
}

TEST_CASE(
	"shape_rule_accessors should read the operation extents in declaration "
	"order",
	"[shape_rule_accessors]"
)
{
	STATIC_CHECK(
		shape_rule_accessors<extent_rule>::operation_extent_count == 2
	);
	STATIC_CHECK(
		shape_rule_accessors<named_rule>::operation_extent_count == 0
	);

	const test_parameters parameters;
	const auto extents =
		shape_rule_accessors<extent_rule>::get_operation_extents(parameters);
	CHECK( extents[0] == 5 );
	CHECK( extents[1] == 7 );
}
