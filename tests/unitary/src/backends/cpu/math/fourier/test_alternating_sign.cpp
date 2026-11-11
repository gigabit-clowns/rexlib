// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/math/fourier/alternating_sign.hpp>

#include <complex>

using namespace rexlib::cpu;

TEST_CASE(
	"alternating_sign should negate a value at a voxel whose coordinates sum "
	"to an odd number",
	"[alternating_sign]"
)
{
	const alternating_sign phase;
	const std::complex<float> value(1, 2);

	CHECK( phase.apply(value, 0, 0, 0) == value );
	CHECK( phase.apply(value, 1, 0, 0) == -value );
	CHECK( phase.apply(value, 1, 1, 0) == value );
	CHECK( phase.apply(value, 2, 3, 4) == -value );
	CHECK( phase.apply(value, 0, 7, 7) == value );
}
