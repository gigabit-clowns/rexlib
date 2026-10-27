// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/em/preprocessing/standardization_map_kernel.hpp>

#include <backends/cpu/kernels/standard_normal.hpp>
#include <backends/cpu/loops/inner_loop_stride_dispatch.hpp>
#include <backends/cpu/loops/linear_index_run.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

using namespace rexlib::cpu;

namespace
{

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();
REXLIB_CONST_CONSTEXPR std::uint64_t seed = 77;

/**
 * @brief Map a contiguous run whose first element has counter @p first.
 */
template <typename T>
std::vector<T> map_run(
	const standardization_map_kernel &kernel,
	const std::vector<T> &values,
	std::ptrdiff_t first = 0
)
{
	std::vector<T> result(values.size(), T(-1));
	kernel(
		std::make_tuple(result.data(), values.data()),
		std::make_tuple(contiguous_stride_tag(), contiguous_stride_tag()),
		values.size(),
		linear_index_run<contiguous_stride_tag>(first, contiguous_stride_tag())
	);
	return result;
}

} // anonymous namespace

TEST_CASE(
	"standardization_map_kernel should standardize a run it keeps whole",
	"[standardization_map_kernel]"
)
{
	const standardization_map_kernel kernel(2, 0.5, -10, 10, seed, 0);

	CHECK(
		map_run(kernel, std::vector<double>{ 1, 2, 3, 6 }) ==
		std::vector<double>{ -0.5, 0, 0.5, 2 }
	);
	CHECK(
		map_run(kernel, std::vector<float>{ 1, 2, 3, 6 }) ==
		std::vector<float>{ -0.5F, 0, 0.5F, 2 }
	);
}

TEST_CASE(
	"standardization_map_kernel should replace what is not finite with the "
	"draw of its counter, wherever it lies in the run",
	"[standardization_map_kernel]"
)
{
	const standardization_map_kernel kernel(
		0,
		1,
		-infinity,
		infinity,
		seed,
		100
	);
	const std::vector<double> values = {
		not_a_number, 1, not_a_number, 2, infinity, 3, -infinity, not_a_number
	};

	const auto result = map_run(kernel, values, 4);

	for (std::size_t i = 0; i < values.size(); ++i)
	{
		INFO( "element " << i );
		const auto kept = i == 1 || i == 3 || i == 5;
		CHECK(
			result[i] ==
			(kept ? values[i] : standard_normal(seed, 104 + i))
		);
	}
}

TEST_CASE(
	"standardization_map_kernel should replace a whole run of what is not "
	"finite",
	"[standardization_map_kernel]"
)
{
	const standardization_map_kernel kernel(0, 1, -3, 3, seed, 0);
	const std::vector<float> values(5, static_cast<float>(not_a_number));

	const auto result = map_run(kernel, values, 7);

	for (std::size_t i = 0; i < values.size(); ++i)
	{
		INFO( "element " << i );
		CHECK(
			result[i] == static_cast<float>(standard_normal(seed, 7 + i))
		);
	}
}

TEST_CASE(
	"standardization_map_kernel should replace a standardized value outside "
	"the bounds",
	"[standardization_map_kernel]"
)
{
	const standardization_map_kernel kernel(1, 1, -2, 3, seed, 10);

	const auto result = map_run(kernel, std::vector<double>{ -5, 1, 4, 5 });

	CHECK( result[0] == standard_normal(seed, 10) );
	CHECK( result[1] == 0 );
	CHECK( result[2] == 3 );
	CHECK( result[3] == standard_normal(seed, 13) );
}

TEST_CASE(
	"standardization_map_kernel should walk runs with strides of their own",
	"[standardization_map_kernel]"
)
{
	const standardization_map_kernel kernel(0, 2, -10, 10, seed, 0);
	const std::vector<double> values = { 1, -1, not_a_number, -1, 3 };
	std::vector<double> result(6, -1);

	// Every other value, into every other second element of the result.
	kernel(
		std::make_tuple(result.data() + 5, values.data()),
		std::make_tuple(std::ptrdiff_t(-2), std::ptrdiff_t(2)),
		3,
		linear_index_run<std::ptrdiff_t>(1, 3)
	);

	CHECK( result[5] == 2 );
	CHECK( result[3] == standard_normal(seed, 4) );
	CHECK( result[1] == 6 );
	CHECK( result[0] == -1 );
	CHECK( result[2] == -1 );
	CHECK( result[4] == -1 );
}
