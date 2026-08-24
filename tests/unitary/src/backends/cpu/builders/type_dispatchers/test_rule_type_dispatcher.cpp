// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <rex/core/dispatch/operation_descriptor.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/platform/constexpr.hpp>
#include <rex/ops/rules/operand_type_rules.hpp>

#include "mock/mock_factory.hpp"

#include <trompeloeil.hpp>

#include <array>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

using namespace rex;
using namespace rex::cpu;

namespace
{

// A backend that cannot execute half precision, which is a narrowing of
// what the operation itself allows rather than a restatement of it.
template <typename... Ts>
struct reject_float16;

template <typename T>
struct reject_float16<T>
	: std::integral_constant<bool, !std::is_same<T, float16_t>::value>
{
};

// The dispatcher names the operation when it rejects one, so the tests
// need something for it to name.
const operation_descriptor& probe_descriptor()
{
	static REXLIB_CONST_CONSTEXPR auto outputs =
		make_operand_names("result");
	static REXLIB_CONST_CONSTEXPR auto inputs =
		make_operand_names("left", "right");
	static const operation_descriptor instance = make_operation_descriptor(
		"rex.test",
		"probe",
		outputs,
		inputs
	);
	return instance;
}

using binary_rule = ops::binary_homogeneous_rule<>;
using abs_rule = ops::unary_real_of_rule<arithmetic_type_domain>;
using convert_rule = ops::converting_rule<>;

} // anonymous namespace

TEST_CASE(
	"rule_type_dispatcher repeats the pivot across a homogeneous rule",
	"[rule_type_dispatcher]"
)
{
	mock_factory factory;

	REQUIRE_CALL(
		factory,
		call(
			std::vector<numerical_type>{ numerical_type::float32 },
			std::vector<numerical_type>{
				numerical_type::float32,
				numerical_type::float32
			}
		)
	).RETURN(nullptr);

	rule_type_dispatcher<binary_rule>::dispatch(
		probe_descriptor(),
		factory,
		std::array<numerical_type, 1>{ numerical_type::float32 },
		std::array<numerical_type, 2>{
			numerical_type::float32,
			numerical_type::float32
		}
	);
}

TEST_CASE(
	"rule_type_dispatcher derives an operand type from its transform",
	"[rule_type_dispatcher]"
)
{
	mock_factory factory;

	// This is what abs needed a hand written dispatcher for: the output is
	// the real counterpart of the input, and the rule already says so.
	REQUIRE_CALL(
		factory,
		call(
			std::vector<numerical_type>{ numerical_type::float64 },
			std::vector<numerical_type>{ numerical_type::complex_float64 }
		)
	).RETURN(nullptr);

	rule_type_dispatcher<abs_rule>::dispatch(
		probe_descriptor(),
		factory,
		std::array<numerical_type, 1>{ numerical_type::float64 },
		std::array<numerical_type, 1>{ numerical_type::complex_float64 }
	);
}

TEST_CASE(
	"rule_type_dispatcher resolves independent pivots for a converting rule",
	"[rule_type_dispatcher]"
)
{
	mock_factory factory;

	REQUIRE_CALL(
		factory,
		call(
			std::vector<numerical_type>{ numerical_type::int16 },
			std::vector<numerical_type>{ numerical_type::float32 }
		)
	).RETURN(nullptr);

	rule_type_dispatcher<convert_rule>::dispatch(
		probe_descriptor(),
		factory,
		std::array<numerical_type, 1>{ numerical_type::int16 },
		std::array<numerical_type, 1>{ numerical_type::float32 }
	);
}

TEST_CASE(
	"rule_type_dispatcher rejects operands the rule does not admit",
	"[rule_type_dispatcher]"
)
{
	using trompeloeil::_;

	mock_factory factory;
	FORBID_CALL(factory, call(_, _));

	CHECK_THROWS_AS(
		rule_type_dispatcher<binary_rule>::dispatch(
			probe_descriptor(),
			factory,
			std::array<numerical_type, 1>{ numerical_type::float32 },
			std::array<numerical_type, 2>{
				numerical_type::float32,
				numerical_type::float64
			}
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"rule_type_dispatcher does not instantiate a rejected combination",
	"[rule_type_dispatcher]"
)
{
	using trompeloeil::_;

	mock_factory factory;
	FORBID_CALL(factory, call(_, _));

	CHECK_THROWS_AS(
		(rule_type_dispatcher<binary_rule, reject_float16>::dispatch(
			probe_descriptor(),
			factory,
			std::array<numerical_type, 1>{ numerical_type::float16 },
			std::array<numerical_type, 2>{
				numerical_type::float16,
				numerical_type::float16
			}
		)),
		std::invalid_argument
	);
}

TEST_CASE(
	"rule_type_dispatcher reports support without throwing",
	"[rule_type_dispatcher]"
)
{
	// Asking whether a configuration can be served is not an error, which
	// is what lets the program manager fall through to another backend
	// instead of an unsupported type escaping from build().
	const std::array<numerical_type, 1> output = { numerical_type::float32 };
	const std::array<numerical_type, 2> matching = {
		numerical_type::float32,
		numerical_type::float32
	};
	const std::array<numerical_type, 2> mismatching = {
		numerical_type::float32,
		numerical_type::float64
	};

	CHECK( rule_type_dispatcher<binary_rule>::is_supported(
		make_span(output.data(), output.size()),
		make_span(matching.data(), matching.size())
	) );

	CHECK( !rule_type_dispatcher<binary_rule>::is_supported(
		make_span(output.data(), output.size()),
		make_span(mismatching.data(), mismatching.size())
	) );
}

TEST_CASE(
	"rule_type_dispatcher reports a backend narrowing as unsupported",
	"[rule_type_dispatcher]"
)
{
	const std::array<numerical_type, 1> output = { numerical_type::float16 };
	const std::array<numerical_type, 2> inputs = {
		numerical_type::float16,
		numerical_type::float16
	};

	// The operation admits float16; this backend does not. The rule is not
	// restated, only intersected.
	CHECK( rule_type_dispatcher<binary_rule>::is_supported(
		make_span(output.data(), output.size()),
		make_span(inputs.data(), inputs.size())
	) );

	CHECK( !(rule_type_dispatcher<binary_rule, reject_float16>::is_supported(
		make_span(output.data(), output.size()),
		make_span(inputs.data(), inputs.size())
	)) );
}

TEST_CASE(
	"rule_type_dispatcher reports mismatching operand counts as unsupported",
	"[rule_type_dispatcher]"
)
{
	const std::array<numerical_type, 1> output = { numerical_type::float32 };
	const std::array<numerical_type, 1> too_few = { numerical_type::float32 };

	CHECK( !rule_type_dispatcher<binary_rule>::is_supported(
		make_span(output.data(), output.size()),
		make_span(too_few.data(), too_few.size())
	) );
}

TEST_CASE(
	"rule_type_dispatcher reports an unknown data type as unsupported",
	"[rule_type_dispatcher]"
)
{
	const std::array<numerical_type, 1> output = { numerical_type::unknown };
	const std::array<numerical_type, 2> inputs = {
		numerical_type::unknown,
		numerical_type::unknown
	};

	CHECK( !rule_type_dispatcher<binary_rule>::is_supported(
		make_span(output.data(), output.size()),
		make_span(inputs.data(), inputs.size())
	) );
}
