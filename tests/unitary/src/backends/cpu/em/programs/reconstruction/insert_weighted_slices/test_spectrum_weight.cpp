// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/programs/reconstruction/insert_weighted_slices/spectrum_weight.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"spectrum_weight should name the inputs of insert_weighted_slices",
	"[spectrum_weight]"
)
{
	STATIC_CHECK( spectrum_weight::extra_index == 3 );
	STATIC_CHECK( spectrum_weight::transform_index == 4 );
	STATIC_CHECK( spectrum_weight::radial_index == 5 );
}

TEST_CASE(
	"spectrum_weight should give the numerator 1 and the denominator the "
	"weight of each stored element",
	"[spectrum_weight]"
)
{
	// A (4, 3) half spectrum of weights, row-major.
	const std::vector<double> weights = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11
	};
	const spectrum_weight::slice_state<double> state(
		spectrum_weight::make_parameters(
			em::insert_weighted_slices_operation(
				em::gridding_window::linear()
			)
		),
		4,
		weights.data(),
		operand_core(2, { 4, 3, 0 }, { 3, 1, 0 })
	);
	std::vector<double> numerator(2);
	std::vector<double> denominator(2);

	spectrum_weight::fill(
		state, 2, -2.0, 1, 2, numerator.data(), denominator.data()
	);

	CHECK( numerator == std::vector<double>{ 1, 1 } );
	CHECK( denominator == std::vector<double>{ 7, 8 } );
}
