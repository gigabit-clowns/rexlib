// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// editorconfig-checker-disable-next-line
#include <backends/cpu/builders/em/reconstruction/slice_insertion_program_builder.hpp>

#include <backends/cpu/builders/em/reconstruction/ctf_weight.hpp>
#include <backends/cpu/builders/em/reconstruction/spectrum_weight.hpp>
#include <backends/cpu/builders/em/reconstruction/unit_weight.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>
#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>
#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

#include "../../../hardware/mock/serial_pool.hpp"
#include "fixtures/slice_insertion_cases.hpp"

#include <complex>
#include <cstddef>
#include <limits>

using namespace rexlib;
using namespace rexlib::cpu;
using namespace rexlib::test;

namespace
{

/**
 * @brief Build a program with a builder and run it over the operands.
 */
template <typename Builder, typename Op, typename T>
insertion_result<T> run_program(
	const Op &operation,
	const insertion_case<T> &data,
	thread_pool &pool,
	bool in_place
)
{
	const insertion_operands<T> operands(data, in_place);
	const Builder builder;
	cpu_command_queue queue(get_serial_pool());
	const auto program = builder.build(
		operation,
		make_span(operands.get_output_descriptors()),
		make_span(operands.get_input_descriptors()),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );
	return operands.execute(dynamic_cast<cpu_program&>(*program), pool);
}

/**
 * @brief Build and run the program of the operation a case names.
 *
 * @param in_place Whether the outputs are the accumulators themselves.
 */
template <typename T>
insertion_result<T> run(
	const insertion_case<T> &data,
	const em::gridding_window &window,
	thread_pool &pool,
	bool in_place = false
)
{
	switch (data.get_variant())
	{
	case insertion_variant::weighted:
		return run_program<
			slice_insertion_program_builder<
				em::insert_weighted_slices_operation,
				spectrum_weight
			>
		>(em::insert_weighted_slices_operation(window), data, pool, in_place);
	case insertion_variant::ctf:
		return run_program<
			slice_insertion_program_builder<
				em::insert_ctf_slices_operation,
				ctf_weight
			>
		>(
			em::insert_ctf_slices_operation(
				window,
				make_optics(),
				case_pixel_size
			),
			data,
			pool,
			in_place
		);
	case insertion_variant::plain:
		break;
	}
	return run_program<
		slice_insertion_program_builder<
			em::insert_slices_operation,
			unit_weight
		>
	>(em::insert_slices_operation(window), data, pool, in_place);
}

} // anonymous namespace

TEST_CASE(
	"the insert_slices program should insert as the contract states, for "
	"either parity and every window",
	"[slice_insertion_program_builder]"
)
{
	const auto sizes = GENERATE(
		size_pair(6, 12), size_pair(5, 7), size_pair(6, 9), size_pair(5, 10)
	);
	const auto window = GENERATE(
		em::gridding_window::nearest(),
		em::gridding_window::linear(),
		em::gridding_window::kaiser_bessel(1.5, 6.0)
	);
	INFO(
		"n = " << sizes.get_slice() << ", N = " << sizes.get_volume()
		<< ", window " << window.get_profile()
	);
	const auto data = make_batch<double>(
		insertion_variant::plain, sizes.get_slice(), sizes.get_volume(), 1
	);

	check_close(
		run(data, window, *get_serial_pool()),
		reference(data, window),
		tolerance_for(window)
	);
}

TEST_CASE(
	"the weighted and CTF insertion programs should weight as the contract "
	"states",
	"[slice_insertion_program_builder]"
)
{
	const auto variant = GENERATE(
		insertion_variant::weighted, insertion_variant::ctf
	);
	const auto sizes = GENERATE(size_pair(6, 12), size_pair(5, 7));
	const auto window = GENERATE(
		em::gridding_window::linear(),
		em::gridding_window::kaiser_bessel(2.0, 8.0)
	);
	INFO(
		variant << ", n = " << sizes.get_slice() << ", N = "
		<< sizes.get_volume() << ", window " << window.get_profile()
	);
	const auto data = make_batch<double>(
		variant, sizes.get_slice(), sizes.get_volume(), 5
	);

	check_close(
		run(data, window, *get_serial_pool()),
		reference(data, window),
		tolerance_for(window)
	);
}

TEST_CASE(
	"the insertion programs should give the same bits on any number of "
	"threads and in place",
	"[slice_insertion_program_builder]"
)
{
	const auto variant = GENERATE(
		insertion_variant::plain,
		insertion_variant::weighted,
		insertion_variant::ctf
	);
	INFO( variant );
	const auto data = make_batch<float>(variant, 8, 16, 2);
	const auto window = em::gridding_window::linear();
	thread_pool threads(3);

	const auto serial = run(data, window, *get_serial_pool());
	const auto parallel = run(data, window, threads);
	const auto in_place = run(data, window, threads, true);

	CHECK( identical(serial, parallel) );
	CHECK( identical(serial, in_place) );
	check_close(serial, reference(data, window), 1e-4);
}

TEST_CASE(
	"the insert_slices program should keep one accumulator per kept batch "
	"element and sum the rest",
	"[slice_insertion_program_builder]"
)
{
	// Two accumulators, each receiving the same three slices through
	// transforms of its own.
	const insertion_case<double> data(
		insertion_variant::plain,
		case_extents(
			{ 2, 1, 8, 8, 5 },
			{ 3, 6, 4 },
			{ 2, 3, 3, 4 },
			{ 3, 4 },
			{}
		),
		3
	);
	const auto window = em::gridding_window::linear();

	check_close(
		run(data, window, *get_serial_pool()),
		reference(data, window),
		1e-9
	);
}

TEST_CASE(
	"the insert_slices program should leave a NaN beyond the cutoff out",
	"[slice_insertion_program_builder]"
)
{
	auto data = make_batch<double>(insertion_variant::plain, 8, 16, 4);
	const auto not_a_number = std::numeric_limits<double>::quiet_NaN();
	// Shells 3 and 4 of every slice are cut off, and the samples at
	// (k_x, k_y) = (4, 0), on shell 4, are NaN.
	for (std::size_t b = 0; b < 3; ++b)
	{
		data.get_radial()[b * 5 + 3] = 0.0;
		data.get_radial()[b * 5 + 4] = 0.0;
		data.get_slices()[b * 8 * 5 + 4] =
			std::complex<double>(not_a_number, not_a_number);
	}
	const auto window = em::gridding_window::linear();

	const auto result = run(data, window, *get_serial_pool());

	for (const auto value : result.get_numerator())
	{
		CHECK( value == value );
	}
	check_close(result, reference(data, window), 1e-9);
}
