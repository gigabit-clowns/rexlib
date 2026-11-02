// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/builders/em/reconstruction/slice_insertion_functor.hpp>

#include <backends/cpu/builders/em/reconstruction/ctf_weight.hpp>
#include <backends/cpu/builders/em/reconstruction/spectrum_weight.hpp>
#include <backends/cpu/builders/em/reconstruction/unit_weight.hpp>
#include <backends/cpu/hardware/functor_program.hpp>
#include <backends/cpu/plans/slice_insertion_layout_plan.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/meta/type_list.hpp>

#include "../../../serial_pool.hpp"
#include "fixtures/slice_insertion_cases.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using namespace rexlib::test;

namespace
{

/**
 * @brief The number of trailing core axes of each input of a variant.
 */
std::vector<std::size_t> core_ranks_of(insertion_variant variant)
{
	switch (variant)
	{
	case insertion_variant::weighted:
		return { 3, 3, 2, 2, 2, 1 };
	case insertion_variant::ctf:
		return { 3, 3, 2, 2, 1, 1 };
	case insertion_variant::plain:
		break;
	}
	return { 3, 3, 2, 2, 1 };
}

/**
 * @brief Run a functor over the operands of a case.
 */
template <typename SampleWeight, typename T>
insertion_result<T> run_functor(
	const insertion_case<T> &data,
	const em::gridding_window &window,
	const typename SampleWeight::parameters_type &parameters,
	const slice_insertion_tuning &tuning,
	thread_pool &pool
)
{
	const insertion_operands<T> operands(data, false);
	const auto ranks = core_ranks_of(data.get_variant());
	slice_insertion_layout_plan plan(
		make_span(operands.get_output_descriptors()),
		make_span(operands.get_input_descriptors()),
		make_span(ranks)
	);
	using functor_type = slice_insertion_functor<T, SampleWeight>;
	functor_type functor(std::move(plan), window, parameters, tuning);

	// The radial weights come last among the inputs, after the two
	// accumulators, the slices and the real inputs of the policy.
	using inputs = type_list_cat_t<
		type_list<std::complex<T>, T, std::complex<T>>,
		typename repeated_type_list<T, SampleWeight::radial_index - 2>::type
	>;
	const auto program = make_functor_program(
		std::move(functor),
		type_list<std::complex<T>, T>(),
		inputs()
	);
	return operands.execute(*program, pool);
}

} // anonymous namespace

TEST_CASE(
	"slice_insertion_functor should insert as the contract states",
	"[slice_insertion_functor]"
)
{
	const auto sizes = GENERATE(size_pair(6, 12), size_pair(5, 7));
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
		insertion_variant::plain, sizes.get_slice(), sizes.get_volume(), 21
	);

	check_close(
		run_functor<unit_weight>(
			data,
			window,
			unit_weight::parameters_type(),
			slice_insertion_tuning(),
			*get_serial_pool()
		),
		reference(data, window),
		tolerance_for(window)
	);
}

TEST_CASE(
	"slice_insertion_functor should give the same bits whatever the tiles",
	"[slice_insertion_functor]"
)
{
	const auto data = make_batch<float>(insertion_variant::weighted, 8, 16, 24);
	const auto window = em::gridding_window::linear();
	thread_pool threads(3);
	const spectrum_weight::parameters_type parameters;

	const auto expected = run_functor<spectrum_weight>(
		data, window, parameters, slice_insertion_tuning(), *get_serial_pool()
	);

	const auto budget = GENERATE(std::size_t(1), std::size_t(1) << 30);
	const auto result = run_functor<spectrum_weight>(
		data,
		window,
		parameters,
		slice_insertion_tuning(budget, 8),
		threads
	);

	CHECK( identical(result, expected) );
}
