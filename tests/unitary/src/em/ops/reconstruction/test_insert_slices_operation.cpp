// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

using specification_list = std::vector<operand_specification>;

const numerical_type complex64 = numerical_type::complex_float32;
const numerical_type float32 = numerical_type::float32;

/**
 * @brief Inputs inserting 3 slices of extent 4 into a volume of extent 8.
 */
specification_list make_inputs()
{
	return {
		operand_specification({ 8, 8, 5 }, complex64),
		operand_specification({ 8, 8, 5 }, float32),
		operand_specification({ 3, 4, 3 }, complex64),
		operand_specification({ 3, 3, 4 }, float32),
		operand_specification({ 3 }, float32)
	};
}

specification_list deduce(const specification_list &inputs)
{
	const insert_slices_operation operation(gridding_window::linear());
	specification_list outputs(2);
	operation.get_signature().deduce(
		operation.get_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs;
}

void accept(
	const specification_list &outputs,
	const specification_list &inputs
)
{
	const insert_slices_operation operation(gridding_window::linear());
	operation.get_signature().accept(
		operation.get_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
}

} // anonymous namespace

TEST_CASE(
	"insert_slices_operation should keep the window it was given",
	"[insert_slices_operation]"
)
{
	const insert_slices_operation operation(
		gridding_window::kaiser_bessel(2.0, 9.0)
	);

	CHECK(
		operation.get_window().get_profile() ==
		gridding_profile::kaiser_bessel
	);
	CHECK( operation.get_window().get_half_width() == 2.0 );
	CHECK( operation.get_window().get_shape() == 9.0 );
}

TEST_CASE(
	"insert_slices_operation should belong to the em component",
	"[insert_slices_operation]"
)
{
	const insert_slices_operation operation(gridding_window::nearest());

	CHECK(
		std::string(operation.get_descriptor().get_component()) ==
		"rexlib.em"
	);
	CHECK(
		std::string(operation.get_descriptor().get_name()) ==
		"insert_slices"
	);
	CHECK(
		operation.get_signature().get_arity() == operation_arity(2, 5)
	);
}

TEST_CASE(
	"insert_slices_operation should give the outputs the shape and type of "
	"the accumulators",
	"[insert_slices_operation]"
)
{
	const auto outputs = deduce(make_inputs());

	CHECK( outputs[0] == operand_specification({ 8, 8, 5 }, complex64) );
	CHECK( outputs[1] == operand_specification({ 8, 8, 5 }, float32) );
}

TEST_CASE(
	"insert_slices_operation should accept odd extents",
	"[insert_slices_operation]"
)
{
	const specification_list inputs = {
		operand_specification({ 7, 7, 4 }, complex64),
		operand_specification({ 7, 7, 4 }, float32),
		operand_specification({ 5, 3 }, complex64),
		operand_specification({ 3, 4 }, float32),
		operand_specification({ 3 }, float32)
	};

	const auto outputs = deduce(inputs);

	CHECK( outputs[0] == operand_specification({ 7, 7, 4 }, complex64) );
	CHECK( outputs[1] == operand_specification({ 7, 7, 4 }, float32) );
}

TEST_CASE(
	"insert_slices_operation should accept float64 operands",
	"[insert_slices_operation]"
)
{
	const specification_list inputs = {
		operand_specification({ 8, 8, 5 }, numerical_type::complex_float64),
		operand_specification({ 8, 8, 5 }, numerical_type::float64),
		operand_specification({ 4, 3 }, numerical_type::complex_float64),
		operand_specification({ 3, 4 }, numerical_type::float64),
		operand_specification({ 3 }, numerical_type::float64)
	};

	const auto outputs = deduce(inputs);

	CHECK(
		outputs[0] ==
		operand_specification({ 8, 8, 5 }, numerical_type::complex_float64)
	);
	CHECK(
		outputs[1] ==
		operand_specification({ 8, 8, 5 }, numerical_type::float64)
	);
}

TEST_CASE(
	"insert_slices_operation should keep the batch of the accumulators and "
	"sum over the rest",
	"[insert_slices_operation]"
)
{
	const specification_list inputs = {
		operand_specification({ 2, 1, 8, 8, 5 }, complex64),
		operand_specification({ 2, 1, 8, 8, 5 }, float32),
		operand_specification({ 3, 4, 3 }, complex64),
		operand_specification({ 2, 3, 3, 4 }, float32),
		operand_specification({ 3 }, float32)
	};

	const auto outputs = deduce(inputs);

	CHECK(
		outputs[0] == operand_specification({ 2, 1, 8, 8, 5 }, complex64)
	);
	CHECK( outputs[1] == operand_specification({ 2, 1, 8, 8, 5 }, float32) );
}

TEST_CASE(
	"insert_slices_operation should accept the accumulators as its outputs",
	"[insert_slices_operation]"
)
{
	const auto inputs = make_inputs();
	const specification_list in_place = { inputs[0], inputs[1] };
	const specification_list batched = {
		operand_specification({ 3, 8, 8, 5 }, complex64),
		operand_specification({ 3, 8, 8, 5 }, float32)
	};

	CHECK_NOTHROW( accept(in_place, inputs) );
	CHECK_THROWS_AS( accept(batched, inputs), std::invalid_argument );
}

TEST_CASE(
	"insert_slices_operation should reject operands of the wrong shape",
	"[insert_slices_operation]"
)
{
	const auto replacement = GENERATE(
		// A volume that is not cubic.
		std::make_pair(0, operand_specification({ 8, 6, 5 }, complex64)),
		// A denominator of another volume.
		std::make_pair(1, operand_specification({ 6, 6, 4 }, float32)),
		// A slice that is not square.
		std::make_pair(2, operand_specification({ 3, 4, 4 }, complex64)),
		// A 3x3 transform.
		std::make_pair(3, operand_specification({ 3, 3, 3 }, float32)),
		// A radial weight for slices of another extent.
		std::make_pair(4, operand_specification({ 4 }, float32)),
		// A batch that does not broadcast.
		std::make_pair(3, operand_specification({ 2, 3, 4 }, float32))
	);

	auto inputs = make_inputs();
	inputs[static_cast<std::size_t>(replacement.first)] = replacement.second;

	CHECK_THROWS_AS( deduce(inputs), std::invalid_argument );
}

TEST_CASE(
	"insert_slices_operation should reject operands of the wrong type",
	"[insert_slices_operation]"
)
{
	const auto replacement = GENERATE(
		// A precision other than that of the denominator.
		std::make_pair(
			2,
			operand_specification(
				{ 3, 4, 3 },
				numerical_type::complex_float64
			)
		),
		// A real slice.
		std::make_pair(2, operand_specification({ 3, 4, 3 }, float32)),
		// A complex denominator.
		std::make_pair(1, operand_specification({ 8, 8, 5 }, complex64)),
		// A half precision denominator.
		std::make_pair(
			1,
			operand_specification({ 8, 8, 5 }, numerical_type::float16)
		)
	);

	auto inputs = make_inputs();
	inputs[static_cast<std::size_t>(replacement.first)] = replacement.second;

	CHECK_THROWS_AS( deduce(inputs), std::invalid_argument );
}
