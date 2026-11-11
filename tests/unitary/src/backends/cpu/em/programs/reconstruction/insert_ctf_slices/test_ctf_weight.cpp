// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/programs/reconstruction/insert_ctf_slices/ctf_weight.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

TEST_CASE(
	"ctf_weight should name the inputs of insert_ctf_slices",
	"[ctf_weight]"
)
{
	STATIC_CHECK( ctf_weight::transform_index == 3 );
	STATIC_CHECK( ctf_weight::extra_index == 4 );
	STATIC_CHECK( ctf_weight::radial_index == 5 );
}

TEST_CASE(
	"ctf_weight should take the optics and the pixel size of the operation",
	"[ctf_weight]"
)
{
	const auto parameters = ctf_weight::make_parameters(
		em::insert_ctf_slices_operation(
			em::gridding_window::linear(),
			em::ctf_optics(200, 2.0, 0.07),
			1.25
		)
	);

	CHECK( parameters.get_optics().get_voltage() == 200 );
	CHECK( parameters.get_pixel_size() == 1.25 );
}

TEST_CASE(
	"ctf_weight should give the numerator the CTF and the denominator its "
	"square",
	"[ctf_weight]"
)
{
	const em::ctf_optics optics(300, 2.7, 0.1);
	const ctf_weight::parameters_type parameters(optics, 1.5);
	// Strided by 2, so that the stride of the core is honoured.
	const std::vector<float> ctf = {
		15000, -1, 12000, -1, 0.4f, -1, 0.2f, -1
	};
	const ctf_weight::slice_state<float> state(
		parameters,
		64,
		ctf.data(),
		operand_core(1, { 4, 0, 0 }, { 2, 0, 0 })
	);
	const contrast_transfer_function_2d<float> expected(
		ctf_phase_coefficients(optics, 1.5, 64),
		15000, 12000, 0.4f, 0.2f
	);
	std::vector<float> numerator(3);
	std::vector<float> denominator(3);

	ctf_weight::fill(
		state, 5, 5.0, 10, 3, numerator.data(), denominator.data()
	);

	for (std::size_t i = 0; i < 3; ++i)
	{
		const auto value = expected(10.0f + static_cast<float>(i), 5.0f);
		CHECK_THAT( numerator[i], WithinAbs(value, 1e-6) );
		CHECK_THAT( denominator[i], WithinAbs(value * value, 1e-6) );
	}
}
