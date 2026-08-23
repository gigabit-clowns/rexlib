// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/ops/policies/axiswise_operation_shape_policy.hpp>
#include <rex/ops/policies/inverse_real_fourier_transform_shape_policy.hpp>
#include <rex/ops/policies/real_fourier_transform_shape_policy.hpp>

#include <rex/core/dispatch/operand_names.hpp>
#include <rex/core/dispatch/operation_descriptor.hpp>
#include <rex/core/platform/constexpr.hpp>

#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;

namespace
{

using shape_type = operation_shape_policy::shape_type;

const operation_descriptor& transform_descriptor()
{
	static REX_CONST_CONSTEXPR auto outputs =
		make_operand_names("spectrum");
	static REX_CONST_CONSTEXPR auto inputs =
		make_operand_names("signal");
	static const operation_descriptor instance =
		make_operation_descriptor(
			"xmipp4.test", "transform", outputs, inputs
		);
	return instance;
}

template <typename Policy>
shape_type deduce_one(const Policy &policy, const shape_type &input)
{
	const std::vector<shape_type> inputs = { input };
	std::vector<shape_type> outputs(1);
	policy.deduce(
		transform_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0];
}

} // anonymous namespace

TEST_CASE(
	"axiswise_operation_shape_policy should leave the shape alone",
	"[fourier_shape_policy]"
)
{
	// A complex to complex transform has as many coefficients as it had
	// samples, so this is the whole of its shape contract.
	const axiswise_operation_shape_policy policy({ 1, 2 });
	CHECK( deduce_one(policy, shape_type{ 4, 8, 16 }) ==
	       shape_type{ 4, 8, 16 } );
}

TEST_CASE(
	"axiswise_operation_shape_policy should reject an axis beyond the rank",
	"[fourier_shape_policy]"
)
{
	const axiswise_operation_shape_policy policy({ 5 });
	CHECK_THROWS_AS(
		deduce_one(policy, shape_type{ 4, 8 }),
		std::out_of_range
	);
}

TEST_CASE(
	"real_fourier_transform_shape_policy should halve only the last "
	"transformed axis",
	"[fourier_shape_policy]"
)
{
	// The conjugate symmetry can only be exploited once, so a two
	// dimensional transform halves one axis and leaves the other whole.
	const real_fourier_transform_shape_policy policy({ 1, 2 });
	CHECK( deduce_one(policy, shape_type{ 4, 8, 16 }) ==
	       shape_type{ 4, 8, 9 } );
}

TEST_CASE(
	"real_fourier_transform_shape_policy should halve an odd axis by "
	"rounding down",
	"[fourier_shape_policy]"
)
{
	const real_fourier_transform_shape_policy policy({ 0 });
	CHECK( deduce_one(policy, shape_type{ 7 }) == shape_type{ 4 } );
	CHECK( deduce_one(policy, shape_type{ 8 }) == shape_type{ 5 } );
}

TEST_CASE(
	"real_fourier_transform_shape_policy should halve the last axis "
	"whatever order the axes were given in",
	"[fourier_shape_policy]"
)
{
	// The axes are sorted on construction, so the halved one is the
	// highest numbered rather than the last one written.
	const real_fourier_transform_shape_policy policy({ 2, 0 });
	CHECK( deduce_one(policy, shape_type{ 8, 4, 16 }) ==
	       shape_type{ 8, 4, 9 } );
}

TEST_CASE(
	"real_fourier_transform_shape_policy should reject transforming no axis",
	"[fourier_shape_policy]"
)
{
	// Without an axis there is no halved one, so the policy would have
	// nothing to describe.
	CHECK_THROWS_AS(
		real_fourier_transform_shape_policy(axis_list()),
		std::invalid_argument
	);
}

TEST_CASE(
	"inverse_real_fourier_transform_shape_policy should restore the axis "
	"according to its parity",
	"[fourier_shape_policy]"
)
{
	// The same operand restores to two different extents, which is why the
	// parity has to be carried rather than worked out.
	const inverse_real_fourier_transform_shape_policy even(
		{ 1 }, signal_parity::even
	);
	CHECK( deduce_one(even, shape_type{ 4, 9 }) == shape_type{ 4, 16 } );

	const inverse_real_fourier_transform_shape_policy odd(
		{ 1 }, signal_parity::odd
	);
	CHECK( deduce_one(odd, shape_type{ 4, 9 }) == shape_type{ 4, 17 } );
}

TEST_CASE(
	"inverse_real_fourier_transform_shape_policy should leave the other "
	"transformed axes alone",
	"[fourier_shape_policy]"
)
{
	const inverse_real_fourier_transform_shape_policy policy(
		{ 0, 1 }, signal_parity::even
	);
	CHECK( deduce_one(policy, shape_type{ 8, 9 }) == shape_type{ 8, 16 } );
}

TEST_CASE(
	"the real transform and its inverse should agree on every extent",
	"[fourier_shape_policy]"
)
{
	// This is the property the pair exists to have: transforming and
	// inverting gets the original extent back, whichever parity it had.
	const real_fourier_transform_shape_policy forward({ 0 });

	for (std::size_t extent = 1; extent <= 32; ++extent)
	{
		const auto stored = deduce_one(forward, shape_type{ extent });

		const inverse_real_fourier_transform_shape_policy backward(
			{ 0 },
			get_signal_parity(extent)
		);

		INFO( "extent " << extent );
		CHECK( deduce_one(backward, stored) == shape_type{ extent } );
	}
}

TEST_CASE(
	"signal parity should be readable and printable",
	"[fourier_shape_policy]"
)
{
	CHECK( get_signal_parity(16) == signal_parity::even );
	CHECK( get_signal_parity(17) == signal_parity::odd );

	const inverse_real_fourier_transform_shape_policy policy(
		{ 0 }, signal_parity::odd
	);
	CHECK( policy.get_parity() == signal_parity::odd );
	CHECK( std::string(to_string(signal_parity::even)) == "even" );
}
