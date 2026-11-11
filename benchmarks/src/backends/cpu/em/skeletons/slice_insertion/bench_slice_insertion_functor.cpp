// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/em/skeletons/slice_insertion/slice_insertion_functor.hpp>

#include <backends/cpu/builders/em/reconstruction/unit_weight.hpp>
#include <backends/cpu/hardware/functor_program.hpp>
#include <backends/cpu/em/skeletons/slice_insertion/slice_insertion_layout_plan.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/meta/type_list.hpp>

// editorconfig-checker-disable-next-line
#include <backends/cpu/em/skeletons/slice_insertion/fixtures/slice_insertion_cases.hpp>

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;
using namespace rexlib::test;

namespace
{

/**
 * @brief Random slices of 256 pixels under uniformly drawn rotations, 128
 * per call into a volume of 512, in float32.
 */
insertion_case<float> make_case()
{
	const std::size_t n = 256;
	const std::size_t big = 512;
	const std::size_t batch = 128;
	const auto h = n / 2 + 1;
	insertion_case<float> data(
		insertion_variant::plain,
		case_extents(
			{ big, big, big / 2 + 1 },
			{ batch, n, h },
			{ batch, 3, 4 },
			{ h },
			{ batch, n, h }
		),
		41
	);

	std::mt19937 engine(42);
	std::normal_distribution<double> normal;
	auto &transforms = data.get_transforms();
	for (std::size_t b = 0; b < batch; ++b)
	{
		double w = normal(engine);
		double x = normal(engine);
		double y = normal(engine);
		double z = normal(engine);
		const auto norm = std::sqrt(w * w + x * x + y * y + z * z);
		w /= norm;
		x /= norm;
		y /= norm;
		z /= norm;
		const std::array<double, 12> rows = {{
			1 - 2 * (y * y + z * z), 2 * (x * y - w * z),
			2 * (x * z + w * y), 0,
			2 * (x * y + w * z), 1 - 2 * (x * x + z * z),
			2 * (y * z - w * x), 0,
			2 * (x * z - w * y), 2 * (y * z + w * x),
			1 - 2 * (x * x + y * y), 0
		}};
		for (std::size_t i = 0; i < rows.size(); ++i)
		{
			transforms[b * rows.size() + i] = static_cast<float>(rows[i]);
		}
	}
	return data;
}

/**
 * @brief Benchmark the insertion of a case under a tuning, in place.
 */
void benchmark_tuning(
	const std::string &name,
	const insertion_case<float> &data,
	const em::gridding_window &window,
	const slice_insertion_tuning &tuning
)
{
	thread_pool pool(thread_pool::get_default_worker_count());
	const insertion_operands<float> operands(data, true);
	const std::vector<std::size_t> ranks = { 3, 3, 2, 2, 1 };
	slice_insertion_layout_plan plan(
		make_span(operands.get_output_descriptors()),
		make_span(operands.get_input_descriptors()),
		make_span(ranks)
	);
	const auto program = make_functor_program(
		slice_insertion_functor<float, unit_weight>(
			std::move(plan),
			window,
			unit_weight::parameters_type(),
			tuning
		),
		type_list<std::complex<float>, float>(),
		type_list<
			std::complex<float>,
			float,
			std::complex<float>,
			float,
			float
		>()
	);

	BENCHMARK(name.c_str())
	{
		operands.run(*program, pool);
	};
}

} // anonymous namespace

TEST_CASE(
	"slice insertion over the size and the number of the tiles",
	"[slice_insertion_tuning]"
)
{
	const auto data = make_case();
	const auto window = em::gridding_window::linear();
	for (const std::size_t budget : { 1u << 20, 1u << 22, 1u << 24 })
	{
		for (const std::size_t per_thread : { 4u, 8u, 16u })
		{
			std::ostringstream name;
			name << budget / 1024 << " KiB, " << per_thread
				<< " per thread, n = 256, N = 512, B = 128";
			benchmark_tuning(
				name.str(),
				data,
				window,
				slice_insertion_tuning(budget, per_thread)
			);
		}
	}
}
