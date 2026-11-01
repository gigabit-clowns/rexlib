// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

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
		operand_specification({ 3, 4, 3 }, float32),
		operand_specification({ 3, 3, 4 }, float32),
		operand_specification({ 3 }, float32)
	};
}

specification_list deduce(const specification_list &inputs)
{
	const insert_weighted_slices_operation operation(
		gridding_window::linear()
	);
	specification_list outputs(2);
	operation.get_signature().deduce(
		operation.get_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs;
}

} // anonymous namespace

TEST_CASE(
	"insert_weighted_slices_operation should keep the window it was given",
	"[insert_weighted_slices_operation]"
)
{
	const insert_weighted_slices_operation operation(
		gridding_window::nearest()
	);

	CHECK( operation.get_window().get_profile() == gridding_profile::box );
}

TEST_CASE(
	"insert_weighted_slices_operation should belong to the em component",
	"[insert_weighted_slices_operation]"
)
{
	const insert_weighted_slices_operation operation(
		gridding_window::linear()
	);

	CHECK(
		std::string(operation.get_descriptor().get_component()) ==
		"rexlib.em"
	);
	CHECK(
		std::string(operation.get_descriptor().get_name()) ==
		"insert_weighted_slices"
	);
	CHECK(
		operation.get_signature().get_arity() == operation_arity(2, 6)
	);
}

TEST_CASE(
	"insert_weighted_slices_operation should give the outputs the shape and "
	"type of the accumulators",
	"[insert_weighted_slices_operation]"
)
{
	const auto outputs = deduce(make_inputs());

	CHECK( outputs[0] == operand_specification({ 8, 8, 5 }, complex64) );
	CHECK( outputs[1] == operand_specification({ 8, 8, 5 }, float32) );
}

TEST_CASE(
	"insert_weighted_slices_operation should broadcast the weights against "
	"the slices",
	"[insert_weighted_slices_operation]"
)
{
	auto inputs = make_inputs();
	inputs[3] = operand_specification({ 4, 3 }, float32);

	CHECK_NOTHROW( deduce(inputs) );
}

TEST_CASE(
	"insert_weighted_slices_operation should reject weights of the wrong "
	"shape or type",
	"[insert_weighted_slices_operation]"
)
{
	const auto weight = GENERATE(
		operand_specification({ 3, 4, 4 }, float32),
		operand_specification({ 3, 5, 3 }, float32),
		operand_specification({ 3, 4, 3 }, complex64),
		operand_specification({ 3, 4, 3 }, numerical_type::float64)
	);

	auto inputs = make_inputs();
	inputs[3] = weight;

	CHECK_THROWS_AS( deduce(inputs), std::invalid_argument );
}
