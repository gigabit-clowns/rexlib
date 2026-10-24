// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <core/operation/type_rule/type_rule_engine.hpp>

#include <rexlib/core/numerical/numerical_type_domain_tags.hpp>
#include <rexlib/core/operation/operand_names.hpp>
#include <rexlib/core/operation/operation_descriptor.hpp>
#include <rexlib/core/operation/type_rule/type_pivot.hpp>
#include <rexlib/core/operation/type_rule/type_pivot_descriptor_table.hpp>
#include <rexlib/core/operation/type_rule/type_slot.hpp>
#include <rexlib/core/operation/type_rule/type_slot_descriptor_table.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include <array>
#include <stdexcept>

using namespace rexlib;
using Catch::Matchers::ContainsSubstring;

namespace
{

struct test_target_type
{
};

using input_pivot = type_list<pivot_from_input<0, floating_point_type_domain>>;

using free_pivots = type_list<
	pivot_from_input<0>,
	pivot_from_output<0, inexact_type_domain>,
	pivot_from_output<1, any_type_domain, 0>
>;

using operation_pivot = type_list<pivot_from_operation<test_target_type>>;

template <typename Pivots>
span<const type_pivot_descriptor> pivots_of()
{
	return type_pivot_descriptor_table<Pivots>::get();
}

template <typename... Slots>
span<const type_slot_descriptor> slots_of()
{
	return type_slot_descriptor_table<type_list<Slots...>>::get();
}

const operation_descriptor& engine_descriptor()
{
	static REXLIB_CONST_CONSTEXPR auto outputs = make_operand_names("result");
	static REXLIB_CONST_CONSTEXPR auto inputs = make_operand_names("value");
	static const operation_descriptor instance =
		make_operation_descriptor("rexlib.test", "engine", outputs, inputs);
	return instance;
}

type_rule_resolution resolve_inputs(numerical_type type)
{
	const std::array<numerical_type, 1> inputs = {{ type }};
	return resolve_pivots(
		pivots_of<input_pivot>(),
		make_span(inputs),
		span<const numerical_type>()
	);
}

type_rule_resolution resolve_free(numerical_type type)
{
	const std::array<numerical_type, 1> inputs = {{ type }};
	return resolve_pivots(
		pivots_of<free_pivots>(),
		make_span(inputs),
		span<const numerical_type>()
	);
}

} // anonymous namespace

TEST_CASE(
	"resolve_pivots should read a pivot from its input",
	"[type_rule_engine]"
)
{
	const auto resolution = resolve_inputs(numerical_type::float32);

	REQUIRE( resolution );
	REQUIRE( resolution.get_pivot_count() == 1 );
	CHECK( resolution.get_pivot(0) == numerical_type::float32 );
}

TEST_CASE(
	"resolve_pivots should reject an input outside the pivot domain",
	"[type_rule_engine]"
)
{
	const auto resolution = resolve_inputs(numerical_type::int32);

	CHECK( resolution.get_status() == type_rule_status::domain_violation );
	CHECK( resolution.get_operand_index() == 0 );
	CHECK_FALSE( resolution.is_output_operand() );
	CHECK( resolution.get_offending_type() == numerical_type::int32 );
}

TEST_CASE(
	"resolve_pivots should reject an input of no concrete type",
	"[type_rule_engine]"
)
{
	CHECK(
		resolve_inputs(numerical_type::unknown).get_status() ==
		type_rule_status::invalid_type
	);
}

TEST_CASE(
	"resolve_pivots should read a pivot the operation supplies",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 1> supplied = {{ numerical_type::int16 }};

	const auto resolution = resolve_pivots(
		pivots_of<operation_pivot>(),
		span<const numerical_type>(),
		make_span(supplied)
	);

	REQUIRE( resolution );
	CHECK( resolution.get_pivot(0) == numerical_type::int16 );
}

TEST_CASE(
	"resolve_pivots should report a pivot the operation does not supply",
	"[type_rule_engine]"
)
{
	const auto resolution = resolve_pivots(
		pivots_of<operation_pivot>(),
		span<const numerical_type>(),
		span<const numerical_type>()
	);

	CHECK( resolution.get_status() == type_rule_status::unresolved_pivot );
}

TEST_CASE(
	"resolve_pivots should leave a free pivot unknown unless it has a "
	"default",
	"[type_rule_engine]"
)
{
	const auto resolution = resolve_free(numerical_type::int8);

	REQUIRE( resolution );
	REQUIRE( resolution.get_pivot_count() == 3 );
	CHECK( resolution.get_pivot(0) == numerical_type::int8 );
	CHECK( resolution.get_pivot(1) == numerical_type::unknown );
	CHECK( resolution.get_pivot(2) == numerical_type::int8 );
}

TEST_CASE(
	"bind_free_pivots should take the free pivots from the outputs",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 2> outputs = {{
		numerical_type::float16,
		numerical_type::uint8
	}};

	const auto resolution = bind_free_pivots(
		pivots_of<free_pivots>(),
		resolve_free(numerical_type::int8),
		make_span(outputs)
	);

	REQUIRE( resolution );
	CHECK( resolution.get_pivot(0) == numerical_type::int8 );
	CHECK( resolution.get_pivot(1) == numerical_type::float16 );
	CHECK( resolution.get_pivot(2) == numerical_type::uint8 );
}

TEST_CASE(
	"bind_free_pivots should reject an output outside the pivot domain",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 2> outputs = {{
		numerical_type::int32,
		numerical_type::int8
	}};

	const auto resolution = bind_free_pivots(
		pivots_of<free_pivots>(),
		resolve_free(numerical_type::int8),
		make_span(outputs)
	);

	CHECK( resolution.get_status() == type_rule_status::domain_violation );
	CHECK( resolution.get_operand_index() == 0 );
	CHECK( resolution.is_output_operand() );
}

TEST_CASE(
	"bind_free_pivots should pass a failed resolution through",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 2> outputs = {{
		numerical_type::float32,
		numerical_type::float32
	}};

	const auto resolution = bind_free_pivots(
		pivots_of<free_pivots>(),
		resolve_free(numerical_type::unknown),
		make_span(outputs)
	);

	CHECK( resolution.get_status() == type_rule_status::invalid_type );
}

TEST_CASE(
	"evaluate_slots should apply each slot to its pivot",
	"[type_rule_engine]"
)
{
	std::array<numerical_type, 3> types;

	const auto resolution = evaluate_slots(
		slots_of<
			slot_same_as<0>,
			slot_complex_of<0>,
			slot_fixed<numerical_type::boolean>
		>(),
		resolve_inputs(numerical_type::float64),
		make_span(types),
		true
	);

	REQUIRE( resolution );
	CHECK( types[0] == numerical_type::float64 );
	CHECK( types[1] == numerical_type::complex_float64 );
	CHECK( types[2] == numerical_type::boolean );
}

TEST_CASE(
	"evaluate_slots should leave the operand of a free pivot unknown",
	"[type_rule_engine]"
)
{
	std::array<numerical_type, 1> types;

	const auto resolution = evaluate_slots(
		slots_of<slot_same_as<1>>(),
		resolve_free(numerical_type::int8),
		make_span(types),
		true
	);

	REQUIRE( resolution );
	CHECK( types[0] == numerical_type::unknown );
}

TEST_CASE(
	"evaluate_slots should reject a pivot its transform cannot map",
	"[type_rule_engine]"
)
{
	std::array<numerical_type, 2> types;

	const auto resolution = evaluate_slots(
		slots_of<slot_same_as<0>, slot_complex_of<0>>(),
		resolve_free(numerical_type::int8),
		make_span(types),
		true
	);

	CHECK( resolution.get_status() == type_rule_status::undefined_transform );
	CHECK( resolution.get_operand_index() == 1 );
	CHECK( resolution.is_output_operand() );
	CHECK( resolution.get_offending_type() == numerical_type::int8 );
}

TEST_CASE(
	"check_slots should accept the types the slots imply",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 2> types = {{
		numerical_type::float32,
		numerical_type::complex_float32
	}};

	CHECK(
		check_slots(
			slots_of<slot_same_as<0>, slot_complex_of<0>>(),
			resolve_inputs(numerical_type::float32),
			make_span(types),
			false
		)
	);
}

TEST_CASE(
	"check_slots should reject a type the slot does not imply",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 1> types = {{ numerical_type::float64 }};

	const auto resolution = check_slots(
		slots_of<slot_same_as<0>>(),
		resolve_inputs(numerical_type::float32),
		make_span(types),
		false
	);

	CHECK( resolution.get_status() == type_rule_status::slot_mismatch );
	CHECK( resolution.get_operand_index() == 0 );
	CHECK_FALSE( resolution.is_output_operand() );
	CHECK( resolution.get_offending_type() == numerical_type::float64 );
}

TEST_CASE(
	"check_slots should report the operand of a free pivot",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 1> types = {{ numerical_type::float32 }};

	CHECK(
		check_slots(
			slots_of<slot_same_as<1>>(),
			resolve_free(numerical_type::int8),
			make_span(types),
			true
		).get_status() == type_rule_status::unresolved_pivot
	);
}

TEST_CASE(
	"check_slots should reject a pivot its transform cannot map",
	"[type_rule_engine]"
)
{
	const std::array<numerical_type, 1> types = {{
		numerical_type::complex_float32
	}};

	CHECK(
		check_slots(
			slots_of<slot_complex_of<0>>(),
			resolve_free(numerical_type::int8),
			make_span(types),
			true
		).get_status() == type_rule_status::undefined_transform
	);
}

TEST_CASE(
	"throw_type_rule_error should name the domain an input violates",
	"[type_rule_engine]"
)
{
	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			resolve_inputs(numerical_type::int32),
			span<const type_slot_descriptor>(),
			pivots_of<input_pivot>()
		),
		ContainsSubstring("rexlib.test.engine: input operand 'value' has "
			"data type int32, which this operation does not accept. "
			"Expected one of: float16, float32, float64.")
	);
}

TEST_CASE(
	"throw_type_rule_error should state the type a slot requires",
	"[type_rule_engine]"
)
{
	const auto slots = slots_of<slot_same_as<0>>();
	const std::array<numerical_type, 1> types = {{ numerical_type::float64 }};
	const auto resolution = check_slots(
		slots,
		resolve_inputs(numerical_type::float32),
		make_span(types),
		true
	);

	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			resolution,
			slots,
			pivots_of<input_pivot>()
		),
		ContainsSubstring("output operand 'result' has data type float64, "
			"but this operation requires float32.")
	);
}

TEST_CASE(
	"throw_type_rule_error should name the relation a transform needs",
	"[type_rule_engine]"
)
{
	const auto slots = slots_of<slot_complex_of<0>>();
	std::array<numerical_type, 1> types;
	const auto resolution = evaluate_slots(
		slots,
		resolve_free(numerical_type::int8),
		make_span(types),
		true
	);

	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			resolution,
			slots,
			pivots_of<free_pivots>()
		),
		ContainsSubstring("the data type of output operand 'result' cannot "
			"be derived from element type int8. It must be the complex "
			"counterpart of one of:")
	);
}

TEST_CASE(
	"throw_type_rule_error should ask for an output it cannot deduce",
	"[type_rule_engine]"
)
{
	const auto slots = slots_of<slot_same_as<1>>();
	const std::array<numerical_type, 1> types = {{ numerical_type::float32 }};
	const auto resolution = check_slots(
		slots,
		resolve_free(numerical_type::int8),
		make_span(types),
		true
	);

	CHECK_THROWS_AS(
		throw_type_rule_error(
			engine_descriptor(),
			resolution,
			slots,
			pivots_of<free_pivots>()
		),
		std::invalid_argument
	);
	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			resolution,
			slots,
			pivots_of<free_pivots>()
		),
		ContainsSubstring("is not determined by the inputs")
	);
}

TEST_CASE(
	"throw_type_rule_error should name the operand a count mismatch "
	"leaves without a slot",
	"[type_rule_engine]"
)
{
	const auto slots = slots_of<slot_same_as<0>>();

	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			type_rule_resolution::failure(
				type_rule_status::operand_count_mismatch,
				0,
				false,
				numerical_type::unknown
			),
			slots,
			pivots_of<input_pivot>()
		),
		ContainsSubstring("input operand 'value' is missing. The typing "
			"rule describes 1 input operand.")
	);
	CHECK_THROWS_WITH(
		throw_type_rule_error(
			engine_descriptor(),
			type_rule_resolution::failure(
				type_rule_status::operand_count_mismatch,
				1,
				true,
				numerical_type::unknown
			),
			slots,
			pivots_of<input_pivot>()
		),
		ContainsSubstring("output operand 1 is not expected.")
	);
}
