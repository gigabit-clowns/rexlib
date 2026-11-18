// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/virtual_row.hpp>

#include <complex>
#include <vector>

using namespace rexlib::cpu;

TEST_CASE(
	"virtual_row should keep what it was given",
	"[virtual_row]"
)
{
	const std::vector<std::complex<float>> samples(4);
	const std::vector<std::complex<float>> phases(4);
	const std::vector<float> radial(3);
	const virtual_row<float> row(
		true,
		5,
		-3.0,
		{{ 1, 2, 3 }},
		{{ 4, 5, 6 }},
		samples.data(),
		2,
		phases.data(),
		std::complex<float>(0, 1),
		radial.data(),
		1,
		2
	);

	CHECK( row.is_negated() );
	CHECK( row.get_stored_row() == 5 );
	CHECK( row.get_k_y() == -3.0 );
	CHECK( row.get_base() == volume_frequency{{ 1, 2, 3 }} );
	CHECK( row.get_step() == volume_frequency{{ 4, 5, 6 }} );
	CHECK( row.get_samples() == samples.data() );
	CHECK( row.get_sample_stride() == 2 );
	CHECK( row.get_x_phases() == phases.data() );
	CHECK( row.get_y_phase() == std::complex<float>(0, 1) );
	CHECK( row.get_radial_weights() == radial.data() );
	CHECK( row.get_radial_stride() == 1 );
	CHECK( row.get_last_shell() == 2 );
}
