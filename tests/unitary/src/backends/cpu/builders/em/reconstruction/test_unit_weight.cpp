// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/em/reconstruction/unit_weight.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"unit_weight should name the transforms and the radial weights of "
	"insert_slices",
	"[unit_weight]"
)
{
	STATIC_CHECK( unit_weight::transform_index == 3 );
	STATIC_CHECK( unit_weight::radial_index == 4 );
	STATIC_CHECK( unit_weight::extra_index == 4 );
}

TEST_CASE(
	"unit_weight should fill every weight with 1",
	"[unit_weight]"
)
{
	const unit_weight::slice_state<float> state(
		unit_weight::make_parameters(
			em::insert_slices_operation(em::gridding_window::linear())
		),
		8,
		nullptr,
		operand_core()
	);
	std::vector<float> numerator(5, 0.0f);
	std::vector<float> denominator(5, 0.0f);

	unit_weight::fill(
		state, 2, -1.0, 3, 4, numerator.data(), denominator.data()
	);

	CHECK( numerator == std::vector<float>{ 1, 1, 1, 1, 0 } );
	CHECK( denominator == std::vector<float>{ 1, 1, 1, 1, 0 } );
}
