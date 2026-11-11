// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/row_insertion_kernel.hpp>

#include <backends/cpu/fourier_shift/alternating_sign.hpp>
#include <backends/cpu/em/programs/reconstruction/insert_slices/unit_weight.hpp>
#include <backends/cpu/em/gridding/triangle_window.hpp>

#include <complex>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using Catch::Matchers::WithinAbs;

namespace
{

using kernel_type = row_insertion_kernel<
	double,
	triangle_window,
	alternating_sign,
	unit_weight
>;
using complex_type = std::complex<double>;

const std::size_t extent = 8;
const std::size_t half = extent / 2 + 1;

// The contiguous core of an (8, 8, 5) half spectrum.
const operand_core volume_core(
	3,
	{ extent, extent, half },
	{ static_cast<std::ptrdiff_t>(extent * half), half, 1 }
);

std::size_t index(std::size_t z, std::size_t y, std::size_t x)
{
	return (z * extent + y) * half + x;
}

} // anonymous namespace

TEST_CASE(
	"row_insertion_kernel should spread one sample over the eight voxels "
	"around it, within the box and the stored half",
	"[row_insertion_kernel]"
)
{
	const triangle_window window;
	const alternating_sign phase;
	kernel_type kernel(window, phase, extent, volume_core, volume_core);

	std::vector<complex_type> numerator(extent * extent * half);
	std::vector<double> denominator(numerator.size());
	kernel.bind(numerator.data(), denominator.data());
	kernel.set_box(frequency_interval(-4, 4), frequency_interval(-4, 4));

	// One sample, k = 0, landing on (1.25, 2.5, -0.75), with value 2.
	const std::vector<complex_type> samples = { complex_type(2, 0) };
	const std::vector<complex_type> phases = { complex_type(1, 0) };
	const std::vector<double> radial = { 1.0 };
	const virtual_row<double> row(
		false,
		0,
		0.0,
		{{ 1.25, 2.5, -0.75 }},
		{{ 0, 0, 0 }},
		samples.data(),
		1,
		phases.data(),
		complex_type(1, 0),
		radial.data(),
		1,
		0
	);
	const unit_weight::slice_state<double> state;

	kernel.insert(row, state, 0, 1);

	// z = -1 is stored at 7, z = 0 at 0; y = 2 and 3; x = 1 and 2.
	CHECK_THAT(
		denominator[index(7, 2, 1)],
		WithinAbs(0.75 * 0.5 * 0.75, 1e-12)
	);
	CHECK_THAT(
		denominator[index(0, 3, 2)],
		WithinAbs(0.25 * 0.5 * 0.25, 1e-12)
	);
	CHECK_THAT(
		numerator[index(7, 2, 1)].real(),
		WithinAbs(2 * 0.75 * 0.5 * 0.75, 1e-12)
	);
	// (x + y + z) = 1 + 2 + 7 is even, so the sign is positive there, and
	// odd at (2, 3, 0).
	CHECK_THAT( numerator[index(2, 3, 0)].real(), WithinAbs(0.0, 1e-12) );
	CHECK_THAT(
		numerator[index(0, 3, 2)].real(),
		WithinAbs(-2 * 0.25 * 0.5 * 0.25, 1e-12)
	);

	double total = 0.0;
	for (const auto weight : denominator)
	{
		total += weight;
	}
	CHECK_THAT( total, WithinAbs(1.0, 1e-12) );
}

TEST_CASE(
	"row_insertion_kernel should write nothing outside its box or below x = 0",
	"[row_insertion_kernel]"
)
{
	const triangle_window window;
	const alternating_sign phase;
	kernel_type kernel(window, phase, extent, volume_core, volume_core);

	std::vector<complex_type> numerator(extent * extent * half);
	std::vector<double> denominator(numerator.size());
	kernel.bind(numerator.data(), denominator.data());
	kernel.set_box(frequency_interval(0, 4), frequency_interval(-4, 4));

	// Straddles x = 0 and z = 0.
	const std::vector<complex_type> samples = { complex_type(1, 0) };
	const std::vector<complex_type> phases = { complex_type(1, 0) };
	const std::vector<double> radial = { 1.0 };
	const virtual_row<double> row(
		false,
		0,
		0.0,
		{{ -0.5, 1.0, -0.5 }},
		{{ 0, 0, 0 }},
		samples.data(),
		1,
		phases.data(),
		complex_type(1, 0),
		radial.data(),
		1,
		0
	);
	const unit_weight::slice_state<double> state;

	kernel.insert(row, state, 0, 1);

	double total = 0.0;
	for (const auto weight : denominator)
	{
		total += weight;
	}
	// Only x = 0 (weight 1/2) and z = 0 (weight 1/2) of the footprint stay.
	CHECK_THAT( total, WithinAbs(0.25, 1e-12) );
	CHECK_THAT( denominator[index(0, 1, 0)], WithinAbs(0.25, 1e-12) );
}

TEST_CASE(
	"row_insertion_kernel should skip a sample beyond the last shell or with "
	"a radial weight of 0",
	"[row_insertion_kernel]"
)
{
	const triangle_window window;
	const alternating_sign phase;
	kernel_type kernel(window, phase, extent, volume_core, volume_core);

	std::vector<complex_type> numerator(extent * extent * half);
	std::vector<double> denominator(numerator.size());
	kernel.bind(numerator.data(), denominator.data());
	kernel.set_box(frequency_interval(-4, 4), frequency_interval(-4, 4));

	const auto not_a_number = std::numeric_limits<double>::quiet_NaN();
	const std::vector<complex_type> samples = {
		complex_type(1, 0), complex_type(not_a_number, 0), complex_type(1, 0)
	};
	const std::vector<complex_type> phases(3, complex_type(1, 0));
	const std::vector<double> radial = { 1.0, 0.0, 1.0 };
	const virtual_row<double> row(
		false,
		0,
		0.0,
		{{ 1, 1, 1 }},
		{{ 1, 0, 0 }},
		samples.data(),
		1,
		phases.data(),
		complex_type(1, 0),
		radial.data(),
		1,
		1
	);
	const unit_weight::slice_state<double> state;

	kernel.insert(row, state, 0, 3);

	double total = 0.0;
	for (const auto weight : denominator)
	{
		total += weight;
	}
	CHECK_THAT( total, WithinAbs(1.0, 1e-12) );
	for (const auto value : numerator)
	{
		CHECK( value == value );
	}
}
