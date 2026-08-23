// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/dispatch/operation_data_type_policy.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <stdexcept>
#include <vector>

using namespace rex;

namespace
{

const operation_descriptor& stub_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("result");
	static REX_CONST_CONSTEXPR auto inputs = make_operand_names("value");
	static const operation_descriptor instance =
		make_operation_descriptor("xmipp4.test", "stub", outputs, inputs);
	return instance;
}

class stub_data_type_policy : public operation_data_type_policy
{
public:
	void deduce(
		const operation_descriptor& /*descriptor*/,
		span<numerical_type> /*canonical_output_types*/,
		span<const numerical_type> /*input_types*/
	) const override {}
};

} // namespace

TEST_CASE(
	"operation_data_type_policy::accept should succeed with zero outputs",
	"[operation_data_type_policy]"
)
{
	const stub_data_type_policy policy;
	CHECK_NOTHROW( policy.accept(stub_descriptor(), {}, {}, {}) );
}

TEST_CASE(
	"operation_data_type_policy::accept should succeed when single user"
	" type equals canonical",
	"[operation_data_type_policy]"
)
{
	const stub_data_type_policy policy;

	const std::vector<numerical_type> user = { numerical_type::float32 };
	const std::vector<numerical_type> canonical = { numerical_type::float32 };

	CHECK_NOTHROW( policy.accept(stub_descriptor(), make_span(user), make_span(canonical), {}) );
}

TEST_CASE(
	"operation_data_type_policy::accept should succeed when all user types"
	" equal canonical",
	"[operation_data_type_policy]"
)
{
	const stub_data_type_policy policy;

	const std::vector<numerical_type> user = {
		numerical_type::float32, numerical_type::int32
	};
	const std::vector<numerical_type> canonical = {
		numerical_type::float32, numerical_type::int32
	};

	CHECK_NOTHROW( policy.accept(stub_descriptor(), make_span(user), make_span(canonical), {}) );
}

TEST_CASE(
	"operation_data_type_policy::accept should throw when user type at"
	" index 0 differs from canonical",
	"[operation_data_type_policy]"
)
{
	const stub_data_type_policy policy;

	const std::vector<numerical_type> user = { numerical_type::float32 };
	const std::vector<numerical_type> canonical = { numerical_type::float64 };

	REQUIRE_THROWS_MATCHES(
		policy.accept(stub_descriptor(), make_span(user), make_span(canonical), {}),
		std::invalid_argument,
		Catch::Matchers::Message(
			"xmipp4.test.stub: output operand 'result' has data type "
			"float32, but this operation deduces float64 from its inputs."
		)
	);
}

TEST_CASE(
	"operation_data_type_policy::accept should throw when user type at a"
	" later index differs from canonical",
	"[operation_data_type_policy]"
)
{
	const stub_data_type_policy policy;

	const std::vector<numerical_type> user = {
		numerical_type::float32,
		numerical_type::int32,
		numerical_type::uint8
	};
	const std::vector<numerical_type> canonical = {
		numerical_type::float32,
		numerical_type::int32,
		numerical_type::int64
	};

	REQUIRE_THROWS_MATCHES(
		policy.accept(stub_descriptor(), make_span(user), make_span(canonical), {}),
		std::invalid_argument,
		Catch::Matchers::Message(
			"xmipp4.test.stub: output operand 2 has data type uint8, but "
			"this operation deduces int64 from its inputs."
		)
	);
}
