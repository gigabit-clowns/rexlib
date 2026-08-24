// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/extremum_locator_kernel.hpp>
#include <backends/cpu/kernels/ordering.hpp>
#include <backends/cpu/load_store.hpp>
#include <backends/cpu/loops/reduction_loop.hpp>

#include <rex/core/meta/type_list.hpp>
#include <rex/core/numerical/fixed_width_float.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

struct maximum_order
{
	template <typename T>
	bool operator()(const T &candidate, const T &best) const noexcept
	{
		return exceeds(candidate, best);
	}
};

struct minimum_order
{
	template <typename T>
	bool operator()(const T &candidate, const T &best) const noexcept
	{
		return precedes(candidate, best);
	}
};

using maximum_locator = extremum_locator_kernel<maximum_order>;
using minimum_locator = extremum_locator_kernel<minimum_order>;

const float not_a_number = std::numeric_limits<float>::quiet_NaN();

// Drive the kernel over a run of elements the way run_reduction_loop does:
// seed from the first, then combine the rest, then finalize.
template <typename Kernel, typename T, std::size_t N>
std::int64_t locate(const Kernel &kernel, const T (&values)[N])
{
	using accumulator_type = typename type_list_element<
		0,
		typename Kernel::template accumulators<
			type_list<std::int64_t>,
			type_list<T>
		>::type
	>::type;

	accumulator_type best = accumulator_type();
	std::int64_t where = 0;
	kernel.seed(best, where, &values[0], std::size_t(0));
	for (std::size_t i = 1; i < N; ++i)
	{
		kernel.combine(best, where, &values[i], i);
	}

	std::int64_t result = -1;
	kernel.finalize(&result, best, where, N);
	return result;
}

} // anonymous namespace

TEST_CASE(
	"extremum_locator_kernel keeps two accumulators typed on the input",
	"[extremum_locator_kernel]"
)
{
	// The output holds an index, so neither accumulator has its type: the
	// extremum is typed on the operand and the place on the index type.
	using accumulators = maximum_locator::accumulators<
		type_list<std::int64_t>,
		type_list<float>
	>::type;

	STATIC_REQUIRE( type_list_size<accumulators>::value == 2 );
	STATIC_REQUIRE(
		std::is_same<
			typename type_list_element<0, accumulators>::type,
			float
		>::value
	);
	STATIC_REQUIRE(
		std::is_same<
			typename type_list_element<1, accumulators>::type,
			std::int64_t
		>::value
	);
}

TEST_CASE(
	"extremum_locator_kernel accumulates half precision in its "
	"computation type",
	"[extremum_locator_kernel]"
)
{
	// An accumulator is typed on the computation type rather than on the
	// storage type, which is what keeps the comparison exact.
	using accumulators = maximum_locator::accumulators<
		type_list<std::int64_t>,
		type_list<float16_t>
	>::type;

	STATIC_REQUIRE(
		std::is_same<
			typename type_list_element<0, accumulators>::type,
			typename element_compute_type<float16_t>::type
		>::value
	);
}

TEST_CASE(
	"extremum_locator_kernel locates the extremum of a run",
	"[extremum_locator_kernel]"
)
{
	const float values[] = { 1.0F, 9.0F, 3.0F, 2.0F };

	CHECK( locate(maximum_locator(), values) == 1 );
	CHECK( locate(minimum_locator(), values) == 0 );
}

TEST_CASE(
	"extremum_locator_kernel keeps the first place of a tie",
	"[extremum_locator_kernel]"
)
{
	const int values[] = { 4, 7, 7, 4 };

	CHECK( locate(maximum_locator(), values) == 1 );
	CHECK( locate(minimum_locator(), values) == 0 );
}

TEST_CASE(
	"extremum_locator_kernel reports the place of a not-a-number",
	"[extremum_locator_kernel]"
)
{
	const float values[] = { 1.0F, not_a_number, 3.0F, 2.0F };

	CHECK( locate(maximum_locator(), values) == 1 );
	CHECK( locate(minimum_locator(), values) == 1 );
}

TEST_CASE(
	"extremum_locator_kernel orders booleans without comparing them",
	"[extremum_locator_kernel]"
)
{
	const bool values[] = { false, false, true, false };

	CHECK( locate(maximum_locator(), values) == 2 );
	CHECK( locate(minimum_locator(), values) == 0 );
}

TEST_CASE(
	"extremum_locator_kernel merges partial accumulators",
	"[extremum_locator_kernel]"
)
{
	// merge is the part of the concept a parallel reduction needs; it must
	// agree with combine about which of two candidates wins.
	const maximum_locator kernel;

	float best = 3.0F;
	std::int64_t where = 7;
	kernel.merge(best, where, 9.0F, std::int64_t(2));
	CHECK( best == 9.0F );
	CHECK( where == 2 );

	// A loser leaves both accumulators untouched.
	kernel.merge(best, where, 1.0F, std::int64_t(5));
	CHECK( best == 9.0F );
	CHECK( where == 2 );
}

TEST_CASE(
	"extremum_locator_kernel writes only the place it found",
	"[extremum_locator_kernel]"
)
{
	const maximum_locator kernel;

	std::int64_t result = -1;
	kernel.finalize(&result, 9.0F, std::int64_t(4), std::size_t(6));

	CHECK( result == 4 );
}

TEST_CASE(
	"extremum_locator_kernel has no identity",
	"[extremum_locator_kernel]"
)
{
	// An extremum of nothing does not exist, so the loop must reject an
	// empty reduction rather than find a neutral element here.
	STATIC_REQUIRE_FALSE(
		has_reduction_identity<
			maximum_locator,
			type_list<float, std::int64_t>
		>::value
	);
}
