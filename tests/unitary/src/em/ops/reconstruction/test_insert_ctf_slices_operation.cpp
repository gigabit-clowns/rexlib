// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

using specification_list = std::vector<operand_specification>;

const numerical_type complex64 = numerical_type::complex_float32;
const numerical_type float32 = numerical_type::float32;

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();

ctf_optics make_optics()
{
	return ctf_optics(300, 2.7, 0.1);
}

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
		operand_specification({ 3, 4 }, float32),
		operand_specification({ 3 }, float32)
	};
}

specification_list deduce(const specification_list &inputs)
{
	const insert_ctf_slices_operation operation(
		gridding_window::linear(),
		make_optics(),
		1.5
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
	"insert_ctf_slices_operation should keep the window, the optics and the "
	"pixel size it was given",
	"[insert_ctf_slices_operation]"
)
{
	const insert_ctf_slices_operation operation(
		gridding_window::nearest(),
		ctf_optics(200, 2.0, 0.07),
		1.25
	);

	CHECK( operation.get_window().get_profile() == gridding_profile::box );
	CHECK( operation.get_optics().get_voltage() == 200 );
	CHECK( operation.get_optics().get_spherical_aberration() == 2.0 );
	CHECK( operation.get_optics().get_amplitude_contrast() == 0.07 );
	CHECK( operation.get_pixel_size() == 1.25 );
}

TEST_CASE(
	"insert_ctf_slices_operation should belong to the em component",
	"[insert_ctf_slices_operation]"
)
{
	const insert_ctf_slices_operation operation(
		gridding_window::linear(),
		make_optics(),
		1.0
	);

	CHECK(
		std::string(operation.get_descriptor().get_component()) ==
		"rexlib.em"
	);
	CHECK(
		std::string(operation.get_descriptor().get_name()) ==
		"insert_ctf_slices"
	);
	CHECK(
		operation.get_signature().get_arity() == operation_arity(2, 6)
	);
}

TEST_CASE(
	"insert_ctf_slices_operation should reject a pixel size that is not "
	"finite and positive",
	"[insert_ctf_slices_operation]"
)
{
	const auto pixel_size = GENERATE(0.0, -1.0, infinity, not_a_number);

	CHECK_THROWS_AS(
		insert_ctf_slices_operation(
			gridding_window::linear(),
			make_optics(),
			pixel_size
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"insert_ctf_slices_operation should give the outputs the shape and type "
	"of the accumulators",
	"[insert_ctf_slices_operation]"
)
{
	const auto outputs = deduce(make_inputs());

	CHECK( outputs[0] == operand_specification({ 8, 8, 5 }, complex64) );
	CHECK( outputs[1] == operand_specification({ 8, 8, 5 }, float32) );
}

TEST_CASE(
	"insert_ctf_slices_operation should broadcast the CTF parameters against "
	"the slices",
	"[insert_ctf_slices_operation]"
)
{
	auto inputs = make_inputs();
	inputs[4] = operand_specification({ 4 }, float32);

	CHECK_NOTHROW( deduce(inputs) );
}

TEST_CASE(
	"insert_ctf_slices_operation should reject CTF parameters of the wrong "
	"shape or type",
	"[insert_ctf_slices_operation]"
)
{
	const auto ctf = GENERATE(
		operand_specification({ 3, 3 }, float32),
		operand_specification({ 3, 5 }, float32),
		operand_specification({ 2, 4 }, float32),
		operand_specification({ 3, 4 }, numerical_type::float64),
		operand_specification({ 3, 4 }, complex64)
	);

	auto inputs = make_inputs();
	inputs[4] = ctf;

	CHECK_THROWS_AS( deduce(inputs), std::invalid_argument );
}
