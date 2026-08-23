// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <backends/cpu/plans/linalg_core_layout_plan.hpp>

#include <rex/core/numerical/fixed_width_float.hpp>

#include <complex>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace rex
{
namespace cpu
{

/**
 * @brief Predicate excluding the types Eigen cannot be handed as a scalar.
 *
 * xmipp4's own float16_t (and its complex counterpart) is not a type Eigen
 * knows how to specialize its numeric traits for; every other type
 * arithmetic_type_domain admits (the fixed-width integers, float, double,
 * and their complex counterparts) is a native Eigen scalar type.
 */
template <typename T>
struct eigen_scalar_support
	: std::integral_constant<
		bool,
		!std::is_same<T, float16_t>::value &&
		!std::is_same<T, std::complex<float16_t>>::value
	>
{
};

/**
 * @brief How an operand's core axes sit in memory.
 *
 * A vector core (rank one) has no major order of its own: a contiguous
 * vector is equally a contiguous row or a contiguous column, so it is
 * tagged separately and treated as compatible with either.
 */
enum class core_layout_kind
{
	row_major_contiguous,
	column_major_contiguous,
	vector_contiguous,
	generic
};

/**
 * @brief Classify how an operand's core axes sit in memory.
 *
 * Defined in eigen_gemm.cpp alongside the rest of this header's
 * implementation, so that no caller needs anything Eigen-related just to
 * classify a core before calling resolve_gemm and friends.
 */
core_layout_kind classify_core_layout(
	const linalg_operand_core &core
) noexcept;

bool is_row_major_compatible(core_layout_kind kind) noexcept;

bool is_column_major_compatible(core_layout_kind kind) noexcept;

/**
 * @brief Pad a rank one core into a rank two column ({extent, 1}).
 *
 * The padding axis has extent one, so its stride is never dereferenced;
 * any value is safe.
 */
linalg_operand_core pad_as_column(const linalg_operand_core &vector_core) noexcept;

/**
 * @brief Pad a rank one core into a rank two row ({1, extent}).
 */
linalg_operand_core pad_as_row(const linalg_operand_core &vector_core) noexcept;

/**
 * @brief Signature every gemm implementation shares, whatever mix of its
 * (M, K, N) extents is compile time and which is Eigen::Dynamic.
 *
 * Resolved once per program (in a kernel's constructor) into a stored
 * function pointer, so a batch loop invoking the kernel many times pays no
 * further dispatch cost.
 */
template <typename T>
using gemm_fn = void (*)(
	T *out,
	const T *left,
	const T *right,
	const linalg_operand_core &out_core,
	const linalg_operand_core &left_core,
	const linalg_operand_core &right_core
);

/**
 * @brief Dispatch a runtime extent to a compile time constant.
 *
 * Mirrors dispatch_single_stride's shape: a handful of sizes are tagged as
 * std::integral_constant<int, extent>, and any other size is passed through
 * to @p f exactly as it arrived, a plain std::size_t. Unlike a stride, a
 * matrix dimension has no single representation that works either way once
 * it reaches Eigen::Matrix, so there is no default baked in here for what
 * an untagged extent means: every visitor built on top of this overloads
 * on the std::size_t case itself and decides there, which is uniformly to
 * name that dimension Eigen::Dynamic, but is this function's caller's call,
 * not this function's.
 */
template <typename F>
auto dispatch_extent(std::size_t extent, F &&f)
{
	switch (extent)
	{
		case 1: return std::forward<F>(f)(std::integral_constant<int, 1>{});
		case 2: return std::forward<F>(f)(std::integral_constant<int, 2>{});
		case 3: return std::forward<F>(f)(std::integral_constant<int, 3>{});
		case 4: return std::forward<F>(f)(std::integral_constant<int, 4>{});
		default: return std::forward<F>(f)(extent);
	}
}

/**
 * @brief Resolve which gemm implementation a (M, K, N) shaped product with
 * the given operand layouts should use, once, up front.
 *
 * Defined in eigen_gemm.cpp, explicitly instantiated there for every type
 * eigen_scalar_support admits: the fixed-size dispatch grid this drives for
 * a type worth it (see eigen_fixed_path_support in the .cpp) instantiates
 * Eigen's GEMM expression templates dozens of times over, which is heavy
 * enough that every caller re-instantiating it on its own would multiply
 * that cost by the number of builders. Declaring it here and defining it
 * once keeps that cost paid exactly once.
 *
 * @tparam T The operand element type.
 * @param m The output and left operand's shared extent along the M axis.
 * @param k The left and right operand's shared extent along the K axis.
 * @param n The output and right operand's shared extent along the N axis.
 * @param out_kind How the output operand's core sits in memory.
 * @param left_kind How the left operand's core sits in memory.
 * @param right_kind How the right operand's core sits in memory.
 * @return The resolved implementation, never null.
 */
template <typename T>
gemm_fn<T> resolve_gemm(
	std::size_t m, std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
);

/**
 * @brief Resolve a (M, K) matrix times a K-vector.
 *
 * @see resolve_gemm
 */
template <typename T>
gemm_fn<T> resolve_gemv(
	std::size_t m, std::size_t k,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
);

/**
 * @brief Resolve a K-vector times a (K, N) matrix.
 *
 * The vector is conjugated, matching NumPy's vecmat: the same convention
 * vecdot follows for its own first operand, treating the vector as the
 * row on the left of an inner-product-like contraction rather than as a
 * plain linear map the way matvec's vector is.
 *
 * @see resolve_gemm
 */
template <typename T>
gemm_fn<T> resolve_vecgemm(
	std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
);

} // namespace cpu
} // namespace rex
