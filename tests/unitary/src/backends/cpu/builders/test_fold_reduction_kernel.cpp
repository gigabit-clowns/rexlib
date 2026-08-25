// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/builders/fold_reduction_kernel.hpp>
#include <backends/cpu/load_store.hpp>
#include <backends/cpu/loops/reduction_loop.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <cstddef>
#include <cstdint>
#include <type_traits>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

// A fold with a neutral element.
struct sum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return accumulator + value;
	}

	template <typename T>
	static T identity() noexcept
	{
		return T(0);
	}
};

// A fold without one, so that an empty reduction has no answer.
struct maximum_fold
{
	template <typename T>
	T operator()(const T &accumulator, const T &value) const noexcept
	{
		return value < accumulator ? accumulator : value;
	}
};

// A lift answering about each element rather than with it.
struct nonzero_lift
{
	template <typename Accumulator, typename T>
	static Accumulator apply(const T &value) noexcept
	{
		return value != T() ? Accumulator(1) : Accumulator(0);
	}
};

using sum_kernel = fold_reduction_kernel<sum_fold>;
using maximum_kernel = fold_reduction_kernel<maximum_fold>;
using counting_kernel = fold_reduction_kernel<sum_fold, nonzero_lift>;

// Drive the kernel over a run the way run_reduction_loop does: seed from the
// first element, then combine the rest, then finalize.
template <typename Out, typename Kernel, typename T, std::size_t N>
Out fold_over(const Kernel &kernel, const T (&values)[N])
{
	using accumulator_type = typename type_list_element<
		0,
		typename Kernel::template accumulators<
			type_list<Out>,
			type_list<T>
		>::type
	>::type;

	accumulator_type accumulator = accumulator_type();
	kernel.seed(accumulator, &values[0], std::size_t(0));
	for (std::size_t i = 1; i < N; ++i)
	{
		kernel.combine(accumulator, &values[i], i);
	}

	Out result = Out();
	kernel.finalize(&result, accumulator, N);
	return result;
}

} // anonymous namespace

TEST_CASE(
	"fold_reduction_kernel keeps one accumulator typed on the output",
	"[fold_reduction_kernel]"
)
{
	// The accumulator is the computation type of the output, so the declared
	// typing rule remains the only thing deciding what a reduction
	// accumulates in.
	using accumulators = sum_kernel::accumulators<
		type_list<double>,
		type_list<float>
	>::type;

	STATIC_REQUIRE( type_list_size<accumulators>::value == 1 );
	STATIC_REQUIRE(
		std::is_same<
			typename type_list_element<0, accumulators>::type,
			double
		>::value
	);
}

TEST_CASE(
	"fold_reduction_kernel accumulates half precision in its computation type",
	"[fold_reduction_kernel]"
)
{
	// Half precision has no arithmetic of its own, so an accumulator over it
	// is widened rather than rounding at every step.
	using accumulators = sum_kernel::accumulators<
		type_list<float16_t>,
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
	"fold_reduction_kernel folds a run of elements",
	"[fold_reduction_kernel]"
)
{
	const float values[] = { 1.0F, 2.0F, 3.0F, 4.0F };

	CHECK( fold_over<float>(sum_kernel(), values) == 10.0F );
	CHECK( fold_over<float>(maximum_kernel(), values) == 4.0F );
}

TEST_CASE(
	"fold_reduction_kernel lifts each element on its way in",
	"[fold_reduction_kernel]"
)
{
	// An operation answering about its elements rather than with them
	// supplies its own lift; here every non-zero counts as one.
	const int values[] = { 0, 7, 0, -3, 5 };

	CHECK( fold_over<std::int64_t>(counting_kernel(), values) == 3 );
}

TEST_CASE(
	"fold_reduction_kernel merges partial accumulators with its fold",
	"[fold_reduction_kernel]"
)
{
	// merge is the part of the concept a parallel reduction needs, and must
	// agree with combine, which is why it is the same fold.
	const sum_kernel kernel;

	double accumulator = 6.0;
	kernel.merge(accumulator, 4.0);

	CHECK( accumulator == 10.0 );
}

TEST_CASE(
	"fold_reduction_kernel offers an identity only when its fold has one",
	"[fold_reduction_kernel]"
)
{
	// Without one, a reduction over no elements is rejected at the one place
	// that can tell the difference rather than failing to compile.
	STATIC_REQUIRE(
		has_reduction_identity<sum_kernel, type_list<double>>::value
	);
	STATIC_REQUIRE_FALSE(
		has_reduction_identity<maximum_kernel, type_list<double>>::value
	);
}

TEST_CASE(
	"fold_reduction_kernel seeds an empty reduction from the identity",
	"[fold_reduction_kernel]"
)
{
	const sum_kernel kernel;

	double accumulator = 7.0;
	kernel.identity(accumulator);

	CHECK( accumulator == 0.0 );
}
