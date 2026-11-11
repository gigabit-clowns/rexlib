// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/ops/optics/ctf_1d_operation.hpp>

#include <rexlib/core/operation/operand_specification.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

using specification_list = std::vector<operand_specification>;

const numerical_type float32 = numerical_type::float32;
const numerical_type float64 = numerical_type::float64;

ctf_optics make_optics()
{
	return ctf_optics(300, 2.7, 0.1);
}

operand_specification deduce(
	std::size_t extent,
	const operand_specification &parameters
)
{
	const ctf_1d_operation operation(extent, make_optics(), 1.5);
	const specification_list inputs = { parameters };
	specification_list outputs(1);
	operation.get_signature().deduce(
		operation.get_descriptor(),
		make_span(outputs),
		make_span(inputs)
	);
	return outputs[0];
}

} // anonymous namespace

TEST_CASE(
	"ctf_1d_operation should keep the extent, the optics and the pixel "
	"size it was given",
	"[ctf_1d_operation]"
)
{
	const ctf_1d_operation operation(8, ctf_optics(200, 2.0, 0.07), 1.25);

	CHECK( operation.get_parameters().get_extent() == 8 );
	CHECK( operation.get_optics().get_voltage() == 200 );
	CHECK( operation.get_optics().get_spherical_aberration() == 2.0 );
	CHECK( operation.get_optics().get_amplitude_contrast() == 0.07 );
	CHECK( operation.get_pixel_size() == 1.25 );
}

TEST_CASE(
	"ctf_1d_operation should belong to the em component",
	"[ctf_1d_operation]"
)
{
	const ctf_1d_operation operation(8, make_optics(), 1.0);

	CHECK(
		std::string(operation.get_descriptor().get_component()) ==
		"rexlib.em"
	);
	CHECK(
		std::string(operation.get_descriptor().get_name()) == "ctf_1d"
	);
	CHECK(
		operation.get_signature().get_arity() == operation_arity(1, 1)
	);
}

TEST_CASE(
	"ctf_1d_operation should reject an extent of 0",
	"[ctf_1d_operation]"
)
{
	CHECK_THROWS_AS(
		ctf_1d_operation(0, make_optics(), 1.0),
		std::invalid_argument
	);
}

TEST_CASE(
	"ctf_1d_operation should reject a pixel size that is not finite and "
	"positive",
	"[ctf_1d_operation]"
)
{
	const auto pixel_size = GENERATE(
		0.0,
		-1.0,
		std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::quiet_NaN()
	);

	CHECK_THROWS_AS(
		ctf_1d_operation(8, make_optics(), pixel_size),
		std::invalid_argument
	);
}

TEST_CASE(
	"ctf_1d_operation should give the output the half spectrum of the "
	"extent, after the batch axes of the parameters",
	"[ctf_1d_operation]"
)
{
	CHECK(
		deduce(8, operand_specification({ 2 }, float32)) ==
		operand_specification({ 5 }, float32)
	);
	CHECK(
		deduce(5, operand_specification({ 2 }, float64)) ==
		operand_specification({ 3 }, float64)
	);
	CHECK(
		deduce(8, operand_specification({ 2, 3, 2 }, float32)) ==
		operand_specification({ 2, 3, 5 }, float32)
	);
}

TEST_CASE(
	"ctf_1d_operation should reject parameters of the wrong shape or type",
	"[ctf_1d_operation]"
)
{
	const auto parameters = GENERATE(
		operand_specification({}, float32),
		operand_specification({ 3, 4 }, float32),
		operand_specification({ 3, 2 }, numerical_type::float16),
		operand_specification({ 3, 2 }, numerical_type::complex_float32),
		operand_specification({ 3, 2 }, numerical_type::int32)
	);

	CHECK_THROWS_AS( deduce(8, parameters), std::invalid_argument );
}
