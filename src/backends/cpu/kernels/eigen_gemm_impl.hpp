// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/**
 * @file
 * @brief Definitions behind eigen_gemm.hpp's resolve_gemm and friends.
 *
 * Not for general use: a builder wanting to resolve a gemm includes
 * eigen_gemm.hpp, which declares the three resolve functions without
 * naming Eigen at all. This header carries what defines them, and exists
 * so that the explicit instantiations can be spread across several
 * translation units instead of crowding into one.
 *
 * They have to be spread because one fixed-path element type carries a
 * three level dispatch grid: resolve_gemm alone names 5x5x5 distinct
 * (M, K, N) shapes, each a separate instantiation of Eigen's product
 * expression templates, with resolve_gemv and resolve_vecgemm adding a
 * 5x5 grid apiece. All four such types in one translation unit peaked
 * near ten gigabytes, which is more than a CI runner has; one type per
 * translation unit brings the worst of them under two and a half, and
 * costs nothing at runtime, since either way the grid is walked once per
 * program and collapses to a stored function pointer.
 *
 * @see REX_INSTANTIATE_EIGEN_GEMM
 */

#include <backends/cpu/kernels/eigen_gemm.hpp>

#include <Eigen/Dense>

#include <complex>
#include <cstddef>
#include <type_traits>

namespace rex
{
namespace cpu
{
namespace detail
{

/**
 * @brief The storage option a (Rows, Cols) Eigen::Matrix may take, some of
 * which may be Eigen::Dynamic.
 *
 * Eigen requires, rather than merely allows, a specific option on a matrix
 * that is really a vector: a row vector (Rows == 1, Cols > 1, including
 * Cols == Eigen::Dynamic) must have the RowMajor bit set, and a column
 * vector (Cols == 1, Rows > 1 or Dynamic) must not. Getting this backwards,
 * or leaving it at the caller's general-matrix choice, fails
 * INVALID_MATRIX_TEMPLATE_PARAMETERS.
 *
 * A 1x1 matrix is both at once, so neither constraint actually applies to
 * it; it is kept at the default (0) explicitly rather than falling through
 * to the general-matrix branch.
 */
template <int Rows, int Cols, bool RowMajor>
struct eigen_matrix_options
	: std::integral_constant<
		int,
		RowMajor ? static_cast<int>(Eigen::RowMajor)
		         : static_cast<int>(Eigen::ColMajor)
	>
{
};

template <int Cols, bool RowMajor>
struct eigen_matrix_options<1, Cols, RowMajor>
	: std::integral_constant<int, static_cast<int>(Eigen::RowMajor)>
{
};

template <int Rows, bool RowMajor>
struct eigen_matrix_options<Rows, 1, RowMajor>
	: std::integral_constant<int, 0>
{
};

template <bool RowMajor>
struct eigen_matrix_options<1, 1, RowMajor>
	: std::integral_constant<int, 0>
{
};

/**
 * @brief Pass a matrix expression through unchanged, or lazily conjugated.
 *
 * The tag mirrors dispatch_extent's visitors: which overload runs is
 * chosen once at compile time by the caller's ConjugateLeft template
 * argument, rather than by a runtime branch that would have to give both
 * branches one common type. vecmat is the only caller that ever asks for
 * the conjugated overload (see resolve_vecgemm), matching how NumPy's
 * vecmat conjugates the vector it treats as a row, the same way vecdot
 * conjugates its own first operand.
 */
template <typename Derived>
inline
const Derived& maybe_conjugate_left(
	const Eigen::MatrixBase<Derived> &m, std::false_type
) noexcept
{
	return m.derived();
}

template <typename Derived>
inline
auto maybe_conjugate_left(
	const Eigen::MatrixBase<Derived> &m, std::true_type
)
{
	return m.derived().conjugate();
}

/**
 * @brief GEMM over row-major-contiguous operands, with a (M, K, N) shape
 * that may be fully fixed, fully dynamic, or any mix of the two.
 *
 * Every one of M, K and N is independently either a small literal or
 * Eigen::Dynamic; Eigen::Map's (pointer, rows, cols) constructor accepts
 * either uniformly (a no-op check against the operand_core's own extents
 * when a dimension is fixed, the only way to know the size at all when it
 * is not), so this one function serves what used to be two: a fully fixed
 * small matrix is exactly as unrolled as it always was, since the compiler
 * still sees Rows/ColsAtCompileTime as literals regardless of which
 * constructor built the Map, and a batch of, say, many points (a large,
 * dynamic M) times one small fixed matrix (K, N) now keeps the small
 * dimensions unrolled instead of falling back to a fully dynamic product
 * merely because one dimension did not fit the old fixed path's range.
 *
 * Callers are what guarantee the M, K, N named here match the operands'
 * actual extents when they are literals: nothing here re-checks that.
 *
 * @tparam ConjugateLeft Whether the left operand is conjugated before the
 * product, as vecmat's vector operand is. Defaults to false, leaving
 * matmul's and matvec's callers unaffected.
 */
template <
	int M, int K, int N, bool RowMajor, typename T, bool ConjugateLeft = false
>
void gemm_call_contiguous(
	T *out,
	const T *left,
	const T *right,
	const linalg_operand_core &out_core,
	const linalg_operand_core &left_core,
	const linalg_operand_core &right_core
)
{
	using left_matrix =
		Eigen::Matrix<T, M, K, eigen_matrix_options<M, K, RowMajor>::value>;
	using right_matrix =
		Eigen::Matrix<T, K, N, eigen_matrix_options<K, N, RowMajor>::value>;
	using out_matrix =
		Eigen::Matrix<T, M, N, eigen_matrix_options<M, N, RowMajor>::value>;

	Eigen::Map<const left_matrix> left_map(
		left,
		static_cast<Eigen::Index>(left_core.get_extent(0)),
		static_cast<Eigen::Index>(left_core.get_extent(1))
	);
	Eigen::Map<const right_matrix> right_map(
		right,
		static_cast<Eigen::Index>(right_core.get_extent(0)),
		static_cast<Eigen::Index>(right_core.get_extent(1))
	);
	Eigen::Map<out_matrix> out_map(
		out,
		static_cast<Eigen::Index>(out_core.get_extent(0)),
		static_cast<Eigen::Index>(out_core.get_extent(1))
	);
	out_map.noalias() = maybe_conjugate_left(
		left_map, std::integral_constant<bool, ConjugateLeft>{}
	) * right_map;
}

/**
 * @brief GEMM over operands of arbitrary, runtime-known strides.
 *
 * The one fallback an operand not contiguous of the major order
 * gemm_call_contiguous assumes uses; always fully dynamic, since there is
 * no fixed shape worth unrolling for a stride pattern the caller cannot
 * even guarantee is packed.
 *
 * @tparam ConjugateLeft See gemm_call_contiguous's parameter of the same
 * name.
 */
template <typename T, bool ConjugateLeft = false>
void gemm_call_strided(
	T *out,
	const T *left,
	const T *right,
	const linalg_operand_core &out_core,
	const linalg_operand_core &left_core,
	const linalg_operand_core &right_core
)
{
	using dynamic_matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
	using stride_t = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>;

	Eigen::Map<const dynamic_matrix, 0, stride_t> left_map(
		left,
		static_cast<Eigen::Index>(left_core.get_extent(0)),
		static_cast<Eigen::Index>(left_core.get_extent(1)),
		stride_t(left_core.get_stride(1), left_core.get_stride(0))
	);
	Eigen::Map<const dynamic_matrix, 0, stride_t> right_map(
		right,
		static_cast<Eigen::Index>(right_core.get_extent(0)),
		static_cast<Eigen::Index>(right_core.get_extent(1)),
		stride_t(right_core.get_stride(1), right_core.get_stride(0))
	);
	Eigen::Map<dynamic_matrix, 0, stride_t> out_map(
		out,
		static_cast<Eigen::Index>(out_core.get_extent(0)),
		static_cast<Eigen::Index>(out_core.get_extent(1)),
		stride_t(out_core.get_stride(1), out_core.get_stride(0))
	);
	out_map.noalias() = maybe_conjugate_left(
		left_map, std::integral_constant<bool, ConjugateLeft>{}
	) * right_map;
}

/**
 * @brief Types worth instantiating the fixed-size dispatch grid for.
 *
 * eigen_scalar_support decides what this backend executes at all; this is
 * the separate, narrower question of what is worth the compile time cost of
 * naming small extents as literals individually. Every other supported type
 * still executes correctly and still keeps whichever operands are
 * contiguous free of a runtime stride, through
 * gemm_call_contiguous<Dynamic, Dynamic, Dynamic, ...> alone, just without
 * any dimension named as a literal.
 *
 * The real types alone are on the grid. A complex scalar costs about the
 * same to compile as a real one and, being four real multiplies per
 * element, gains proportionally less from having its extents unrolled;
 * together the two of them were the larger half of a grid that no CI
 * runner could compile four translation units of at once. The small fixed
 * shapes this exists for, the 3x3 and 4x4 of a rotation or a homogeneous
 * transform, are real in every case this codebase has.
 */
template <typename T>
struct eigen_fixed_path_support
	: std::integral_constant<
		bool,
		std::is_same<T, float>::value ||
		std::is_same<T, double>::value
	>
{
};

/**
 * @brief Whether every one of a batch of operand kinds is row-major
 * contiguous (a vector core is compatible with either major order).
 *
 * What decides between gemm_call_contiguous and gemm_call_strided: only
 * row-major is fast-pathed, since this codebase's layouts default to it,
 * and a column-major operand is still correct, just through the strided
 * path, which halves the size of the dispatch grid below for a case this
 * backend does not expect to be common.
 */
inline bool all_row_major_compatible(
	core_layout_kind a, core_layout_kind b, core_layout_kind c
) noexcept
{
	return is_row_major_compatible(a) &&
	       is_row_major_compatible(b) &&
	       is_row_major_compatible(c);
}

/**
 * @brief Pick the N instantiation of gemm_call_contiguous for an M and a K
 * already known.
 *
 * M and K are this class template's own parameters, already concrete by the
 * time gemm_call_contiguous is named below: nothing here is derived through
 * decltype from a generic lambda's deduced argument, unlike an earlier
 * version of this dispatch, which some MSVC toolsets fail to compile (a
 * class template instantiation failure reached however M was carried,
 * fixed size or not, real or complex scalar) even though the exact same
 * instantiation compiles when named directly, as it is here.
 *
 * The two overloads are dispatch_extent's two possible calls for N: a
 * literal names itself, and a still-runtime size (past what is worth
 * naming, see eigen_fixed_path_support and dispatch_extent) names N
 * Eigen::Dynamic instead of clamping to the largest literal, which is what
 * lets M and K stay unrolled even when this one is large.
 */
template <int M, int K, typename T>
struct select_gemm_n_visitor
{
	template <int N>
	gemm_fn<T> operator()(std::integral_constant<int, N>) const noexcept
	{
		return &gemm_call_contiguous<M, K, N, true, T>;
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return &gemm_call_contiguous<M, K, Eigen::Dynamic, true, T>;
	}
};

template <int M, int K, typename T>
gemm_fn<T> select_gemm_n(std::size_t n) noexcept
{
	return dispatch_extent(n, select_gemm_n_visitor<M, K, T>{});
}

/**
 * @brief Pick the K instantiation of select_gemm_n for an M already known.
 *
 * @see select_gemm_n_visitor
 */
template <int M, typename T>
struct select_gemm_kn_visitor
{
	std::size_t n;

	template <int K>
	gemm_fn<T> operator()(std::integral_constant<int, K>) const noexcept
	{
		return select_gemm_n<M, K, T>(n);
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return select_gemm_n<M, Eigen::Dynamic, T>(n);
	}
};

template <int M, typename T>
gemm_fn<T> select_gemm_kn(std::size_t k, std::size_t n) noexcept
{
	return dispatch_extent(k, select_gemm_kn_visitor<M, T>{ n });
}

/**
 * @brief Pick the M instantiation of select_gemm_kn.
 *
 * @see select_gemm_n_visitor
 */
template <typename T>
struct select_gemm_mkn_visitor
{
	std::size_t k;
	std::size_t n;

	template <int M>
	gemm_fn<T> operator()(std::integral_constant<int, M>) const noexcept
	{
		return select_gemm_kn<M, T>(k, n);
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return select_gemm_kn<Eigen::Dynamic, T>(k, n);
	}
};

template <typename T>
gemm_fn<T> select_gemm_mkn(std::size_t m, std::size_t k, std::size_t n) noexcept
{
	return dispatch_extent(m, select_gemm_mkn_visitor<T>{ k, n });
}

/**
 * @brief Pick the K instantiation of gemm_call_contiguous for an M already
 * known. N is always one: the right and output operands of a matrix-vector
 * product are vectors.
 *
 * @see select_gemm_n_visitor
 */
template <int M, typename T>
struct select_gemv_k_visitor
{
	template <int K>
	gemm_fn<T> operator()(std::integral_constant<int, K>) const noexcept
	{
		return &gemm_call_contiguous<M, K, 1, true, T>;
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return &gemm_call_contiguous<M, Eigen::Dynamic, 1, true, T>;
	}
};

template <int M, typename T>
gemm_fn<T> select_gemv_k(std::size_t k) noexcept
{
	return dispatch_extent(k, select_gemv_k_visitor<M, T>{});
}

/**
 * @brief Pick the M instantiation of select_gemv_k.
 *
 * @see select_gemm_n_visitor
 */
template <typename T>
struct select_gemv_mk_visitor
{
	std::size_t k;

	template <int M>
	gemm_fn<T> operator()(std::integral_constant<int, M>) const noexcept
	{
		return select_gemv_k<M, T>(k);
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return select_gemv_k<Eigen::Dynamic, T>(k);
	}
};

template <typename T>
gemm_fn<T> select_gemv_mk(std::size_t m, std::size_t k) noexcept
{
	return dispatch_extent(m, select_gemv_mk_visitor<T>{ k });
}

/**
 * @brief Pick the N instantiation of gemm_call_contiguous for a K already
 * known. M is always one: the left and output operands of a vector-matrix
 * product are vectors. Left is always conjugated: see resolve_vecgemm.
 *
 * @see select_gemm_n_visitor
 */
template <int K, typename T>
struct select_vecgemm_n_visitor
{
	template <int N>
	gemm_fn<T> operator()(std::integral_constant<int, N>) const noexcept
	{
		return &gemm_call_contiguous<1, K, N, true, T, true>;
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return &gemm_call_contiguous<1, K, Eigen::Dynamic, true, T, true>;
	}
};

template <int K, typename T>
gemm_fn<T> select_vecgemm_n(std::size_t n) noexcept
{
	return dispatch_extent(n, select_vecgemm_n_visitor<K, T>{});
}

/**
 * @brief Pick the K instantiation of select_vecgemm_n.
 *
 * @see select_gemm_n_visitor
 */
template <typename T>
struct select_vecgemm_kn_visitor
{
	std::size_t n;

	template <int K>
	gemm_fn<T> operator()(std::integral_constant<int, K>) const noexcept
	{
		return select_vecgemm_n<K, T>(n);
	}

	gemm_fn<T> operator()(std::size_t) const noexcept
	{
		return select_vecgemm_n<Eigen::Dynamic, T>(n);
	}
};

template <typename T>
gemm_fn<T> select_vecgemm_kn(std::size_t k, std::size_t n) noexcept
{
	return dispatch_extent(k, select_vecgemm_kn_visitor<T>{ n });
}

template <typename T>
gemm_fn<T> resolve_gemm(
	std::size_t /*m*/, std::size_t /*k*/, std::size_t /*n*/,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::false_type /* eigen_fixed_path_support<T> */
) noexcept
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return &gemm_call_contiguous<
			Eigen::Dynamic, Eigen::Dynamic, Eigen::Dynamic, true, T
		>;
	}

	return &gemm_call_strided<T>;
}

template <typename T>
gemm_fn<T> resolve_gemm(
	std::size_t m, std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::true_type /* eigen_fixed_path_support<T> */
)
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return select_gemm_mkn<T>(m, k, n);
	}

	return &gemm_call_strided<T>;
}

template <typename T>
gemm_fn<T> resolve_gemv(
	std::size_t /*m*/, std::size_t /*k*/,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::false_type /* eigen_fixed_path_support<T> */
) noexcept
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return &gemm_call_contiguous<Eigen::Dynamic, Eigen::Dynamic, 1, true, T>;
	}

	return &gemm_call_strided<T>;
}

template <typename T>
gemm_fn<T> resolve_gemv(
	std::size_t m, std::size_t k,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::true_type /* eigen_fixed_path_support<T> */
)
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return select_gemv_mk<T>(m, k);
	}

	return &gemm_call_strided<T>;
}

template <typename T>
gemm_fn<T> resolve_vecgemm(
	std::size_t /*k*/, std::size_t /*n*/,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::false_type /* eigen_fixed_path_support<T> */
) noexcept
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return &gemm_call_contiguous<
			1, Eigen::Dynamic, Eigen::Dynamic, true, T, true
		>;
	}

	return &gemm_call_strided<T, true>;
}

template <typename T>
gemm_fn<T> resolve_vecgemm(
	std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind,
	std::true_type /* eigen_fixed_path_support<T> */
)
{
	if (all_row_major_compatible(out_kind, left_kind, right_kind))
	{
		return select_vecgemm_kn<T>(k, n);
	}

	return &gemm_call_strided<T, true>;
}

} // namespace detail

template <typename T>
gemm_fn<T> resolve_gemm(
	std::size_t m, std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
)
{
	return detail::resolve_gemm<T>(
		m, k, n, out_kind, left_kind, right_kind,
		detail::eigen_fixed_path_support<T>{}
	);
}

template <typename T>
gemm_fn<T> resolve_gemv(
	std::size_t m, std::size_t k,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
)
{
	return detail::resolve_gemv<T>(
		m, k, out_kind, left_kind, right_kind,
		detail::eigen_fixed_path_support<T>{}
	);
}

template <typename T>
gemm_fn<T> resolve_vecgemm(
	std::size_t k, std::size_t n,
	core_layout_kind out_kind,
	core_layout_kind left_kind,
	core_layout_kind right_kind
)
{
	return detail::resolve_vecgemm<T>(
		k, n, out_kind, left_kind, right_kind,
		detail::eigen_fixed_path_support<T>{}
	);
}

} // namespace cpu
} // namespace rex

/**
 * @brief Instantiate the three gemm resolvers for one element type.
 *
 * Spelled once per translation unit named eigen_gemm_<type>.cpp, which is
 * the whole of what those files do: a fixed-path type's grid is expensive
 * enough to compile that it wants a translation unit to itself.
 *
 * @param T The element type, which must satisfy eigen_scalar_support.
 */
#define REX_INSTANTIATE_EIGEN_GEMM(T)                                      \
	template gemm_fn<T> resolve_gemm<T>(                                      \
		std::size_t, std::size_t, std::size_t,                                \
		core_layout_kind, core_layout_kind, core_layout_kind                  \
	);                                                                        \
	template gemm_fn<T> resolve_gemv<T>(                                      \
		std::size_t, std::size_t,                                             \
		core_layout_kind, core_layout_kind, core_layout_kind                  \
	);                                                                        \
	template gemm_fn<T> resolve_vecgemm<T>(                                   \
		std::size_t, std::size_t,                                             \
		core_layout_kind, core_layout_kind, core_layout_kind                  \
	)
