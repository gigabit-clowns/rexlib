// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/kernels/eigen_gemm.hpp>

#include <complex>
#include <cstddef>
#include <utility>
#include <vector>

using namespace rexlib::cpu;

TEST_CASE(
	"classify_core_layout recognises a contiguous vector",
	"[eigen_gemm]"
)
{
	const linalg_operand_core contiguous(1, { 5, 0 }, { 1, 0 });
	CHECK(
		classify_core_layout(contiguous) == core_layout_kind::vector_contiguous
	);

	const linalg_operand_core strided(1, { 5, 0 }, { 2, 0 });
	CHECK( classify_core_layout(strided) == core_layout_kind::generic );
}

TEST_CASE(
	"classify_core_layout recognises a row-major contiguous matrix",
	"[eigen_gemm]"
)
{
	const linalg_operand_core core(2, { 2, 3 }, { 3, 1 });
	CHECK(
		classify_core_layout(core) == core_layout_kind::row_major_contiguous
	);
}

TEST_CASE(
	"classify_core_layout recognises a column-major contiguous matrix",
	"[eigen_gemm]"
)
{
	const linalg_operand_core core(2, { 2, 3 }, { 1, 2 });
	CHECK(
		classify_core_layout(core) ==
		core_layout_kind::column_major_contiguous
	);
}

TEST_CASE(
	"classify_core_layout falls back to generic for an arbitrary matrix "
	"stride",
	"[eigen_gemm]"
)
{
	const linalg_operand_core core(2, { 2, 3 }, { 5, 2 });
	CHECK( classify_core_layout(core) == core_layout_kind::generic );
}

TEST_CASE(
	"is_row_major_compatible and is_column_major_compatible agree with "
	"every core_layout_kind",
	"[eigen_gemm]"
)
{
	CHECK( is_row_major_compatible(core_layout_kind::row_major_contiguous) );
	CHECK(
		!is_column_major_compatible(core_layout_kind::row_major_contiguous)
	);

	CHECK(
		is_column_major_compatible(core_layout_kind::column_major_contiguous)
	);
	CHECK(
		!is_row_major_compatible(core_layout_kind::column_major_contiguous)
	);

	// A vector core has no major order of its own, so it is compatible with
	// either.
	CHECK( is_row_major_compatible(core_layout_kind::vector_contiguous) );
	CHECK( is_column_major_compatible(core_layout_kind::vector_contiguous) );

	CHECK( !is_row_major_compatible(core_layout_kind::generic) );
	CHECK( !is_column_major_compatible(core_layout_kind::generic) );
}

TEST_CASE(
	"pad_as_column turns a rank one core into a rank two one-column core",
	"[eigen_gemm]"
)
{
	const linalg_operand_core vector_core(1, { 5, 0 }, { 2, 0 });
	const auto padded = pad_as_column(vector_core);

	CHECK( padded.get_rank() == 2 );
	CHECK( padded.get_extent(0) == 5 );
	CHECK( padded.get_extent(1) == 1 );
	CHECK( padded.get_stride(0) == 2 );
}

TEST_CASE(
	"pad_as_row turns a rank one core into a rank two one-row core",
	"[eigen_gemm]"
)
{
	const linalg_operand_core vector_core(1, { 5, 0 }, { 2, 0 });
	const auto padded = pad_as_row(vector_core);

	CHECK( padded.get_rank() == 2 );
	CHECK( padded.get_extent(0) == 1 );
	CHECK( padded.get_extent(1) == 5 );
	CHECK( padded.get_stride(1) == 2 );
}

namespace
{

/**
 * @brief Distinguishes which of dispatch_extent's two calls a functor
 * received, folding both into one type so the switch it drives can share a
 * single return type.
 */
struct extent_probe
{
	template <int N>
	std::pair<bool, std::size_t> operator()(
		std::integral_constant<int, N>
	) const noexcept
	{
		return { true, static_cast<std::size_t>(N) };
	}

	std::pair<bool, std::size_t> operator()(std::size_t value) const noexcept
	{
		return { false, value };
	}
};

} // namespace

TEST_CASE(
	"dispatch_extent tags extents one through four as a compile time "
	"constant",
	"[eigen_gemm]"
)
{
	for (std::size_t extent = 1; extent <= 4; ++extent)
	{
		const auto result = dispatch_extent(extent, extent_probe{});
		CHECK( result.first );
		CHECK( result.second == extent );
	}
}

TEST_CASE(
	"dispatch_extent passes any other extent through as a plain std::size_t",
	"[eigen_gemm]"
)
{
	for (const std::size_t extent : { std::size_t(0), std::size_t(5),
	                                   std::size_t(100) })
	{
		const auto result = dispatch_extent(extent, extent_probe{});
		CHECK( !result.first );
		CHECK( result.second == extent );
	}
}

namespace
{

linalg_operand_core row_major_core(std::size_t rows, std::size_t cols)
{
	return linalg_operand_core(
		2, { rows, cols }, { static_cast<std::ptrdiff_t>(cols), 1 }
	);
}

linalg_operand_core column_major_core(std::size_t rows, std::size_t cols)
{
	return linalg_operand_core(
		2, { rows, cols }, { 1, static_cast<std::ptrdiff_t>(rows) }
	);
}

} // namespace

TEST_CASE(
	"resolve_gemm computes a fully fixed size row-major product",
	"[eigen_gemm]"
)
{
	const std::vector<float> left = { 1, 2, 3, 4, 5, 6 }; // 2x3
	const std::vector<float> right = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
	}; // 3x4
	std::vector<float> out(8, 0.0f);

	const auto out_core = row_major_core(2, 4);
	const auto left_core = row_major_core(2, 3);
	const auto right_core = row_major_core(3, 4);

	const auto gemm = resolve_gemm<float>(
		2, 3, 4,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous
	);
	REQUIRE( gemm != nullptr );

	gemm(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	const std::vector<float> expected = { 38, 44, 50, 56, 83, 98, 113, 128 };
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_gemm computes a product whose M extent is left dynamic",
	"[eigen_gemm]"
)
{
	// M = 5 falls outside the fixed literal range, so this exercises
	// select_gemm_mkn's Eigen::Dynamic branch for M while K and N (2) stay
	// fixed. right selects the left matrix's own two columns, so the
	// expected product is easy to state without summing anything by hand.
	const std::vector<float> left = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	}; // 5x3
	const std::vector<float> right = { 1, 0, 0, 1, 0, 0 }; // 3x2
	std::vector<float> out(10, 0.0f);

	const auto out_core = row_major_core(5, 2);
	const auto left_core = row_major_core(5, 3);
	const auto right_core = row_major_core(3, 2);

	const auto gemm = resolve_gemm<float>(
		5, 3, 2,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous
	);
	REQUIRE( gemm != nullptr );

	gemm(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	const std::vector<float> expected = {
		1, 2, 4, 5, 7, 8, 10, 11, 13, 14
	};
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_gemm falls back to the strided path for a column-major operand",
	"[eigen_gemm]"
)
{
	// Same logical product as the first resolve_gemm case, but right is
	// stored column-major, which is not row-major compatible and so must
	// take gemm_call_strided instead of the contiguous fast path.
	const std::vector<float> left = { 1, 2, 3, 4, 5, 6 }; // 2x3, row-major
	const std::vector<float> right = {
		1, 5, 9, 2, 6, 10, 3, 7, 11, 4, 8, 12
	}; // 3x4, column-major, same logical values as the row-major case
	std::vector<float> out(8, 0.0f);

	const auto out_core = row_major_core(2, 4);
	const auto left_core = row_major_core(2, 3);
	const auto right_core = column_major_core(3, 4);

	CHECK(
		classify_core_layout(right_core) ==
		core_layout_kind::column_major_contiguous
	);

	const auto gemm = resolve_gemm<float>(
		2, 3, 4,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::column_major_contiguous
	);
	REQUIRE( gemm != nullptr );

	gemm(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	const std::vector<float> expected = { 38, 44, 50, 56, 83, 98, 113, 128 };
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_gemv computes a matrix times a vector",
	"[eigen_gemm]"
)
{
	const std::vector<float> left = { 1, 2, 3, 4, 5, 6 }; // 3x2
	const std::vector<float> right = { 10, 20 };
	std::vector<float> out(3, 0.0f);

	const auto out_core = pad_as_column(linalg_operand_core(1, { 3, 0 }, { 1, 0 }));
	const auto left_core = row_major_core(3, 2);
	const auto right_core =
		pad_as_column(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));

	const auto gemv = resolve_gemv<float>(
		3, 2,
		core_layout_kind::vector_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::vector_contiguous
	);
	REQUIRE( gemv != nullptr );

	gemv(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	const std::vector<float> expected = { 50, 110, 170 };
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_vecgemm computes a vector times a matrix",
	"[eigen_gemm]"
)
{
	const std::vector<float> left = { 1, 2 };
	const std::vector<float> right = { 1, 2, 3, 4, 5, 6 }; // 2x3
	std::vector<float> out(3, 0.0f);

	const auto out_core = pad_as_row(linalg_operand_core(1, { 3, 0 }, { 1, 0 }));
	const auto left_core =
		pad_as_row(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));
	const auto right_core = row_major_core(2, 3);

	const auto vecgemm = resolve_vecgemm<float>(
		2, 3,
		core_layout_kind::vector_contiguous,
		core_layout_kind::vector_contiguous,
		core_layout_kind::row_major_contiguous
	);
	REQUIRE( vecgemm != nullptr );

	vecgemm(
		out.data(), left.data(), right.data(), out_core, left_core, right_core
	);

	const std::vector<float> expected = { 9, 12, 15 };
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_gemm does not conjugate its left operand",
	"[eigen_gemm]"
)
{
	// Real numbers are their own conjugate, so this needs complex values to
	// tell "conjugated" apart from "not"; right is the identity, so the
	// product is just left, unconjugated, if it is correct at all.
	using complex = std::complex<float>;
	const std::vector<complex> left = {
		{ 3, 4 }, { 1, -2 }, { 5, 1 }, { 2, -3 }
	}; // 2x2
	const std::vector<complex> right = { 1, 0, 0, 1 }; // 2x2 identity
	std::vector<complex> out(4);

	const auto out_core = row_major_core(2, 2);
	const auto left_core = row_major_core(2, 2);
	const auto right_core = row_major_core(2, 2);

	const auto gemm = resolve_gemm<complex>(
		2, 2, 2,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::row_major_contiguous
	);
	REQUIRE( gemm != nullptr );

	gemm(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	CHECK( out == left );
}

TEST_CASE(
	"resolve_gemv does not conjugate its matrix operand",
	"[eigen_gemm]"
)
{
	// right is a one-hot vector, so the product picks out left's first
	// column verbatim if the matrix is left alone, or its conjugate if it
	// is not.
	using complex = std::complex<float>;
	const std::vector<complex> left = {
		{ 3, 4 }, { 1, -2 }, { 5, 1 }, { 2, -3 }
	}; // 2x2
	const std::vector<complex> right = { 1, 0 };
	std::vector<complex> out(2);

	const auto out_core =
		pad_as_column(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));
	const auto left_core = row_major_core(2, 2);
	const auto right_core =
		pad_as_column(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));

	const auto gemv = resolve_gemv<complex>(
		2, 2,
		core_layout_kind::vector_contiguous,
		core_layout_kind::row_major_contiguous,
		core_layout_kind::vector_contiguous
	);
	REQUIRE( gemv != nullptr );

	gemv(out.data(), left.data(), right.data(), out_core, left_core, right_core);

	const std::vector<complex> expected = { { 3, 4 }, { 5, 1 } };
	CHECK( out == expected );
}

TEST_CASE(
	"resolve_vecgemm conjugates its vector operand",
	"[eigen_gemm]"
)
{
	// right is the identity, so the product is left's conjugate if vecmat's
	// NumPy-matching convention is honoured, or left itself if it is not.
	using complex = std::complex<float>;
	const std::vector<complex> left = { { 3, 4 }, { 1, -2 } };
	const std::vector<complex> right = { 1, 0, 0, 1 }; // 2x2 identity
	std::vector<complex> out(2);

	const auto out_core =
		pad_as_row(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));
	const auto left_core =
		pad_as_row(linalg_operand_core(1, { 2, 0 }, { 1, 0 }));
	const auto right_core = row_major_core(2, 2);

	const auto vecgemm = resolve_vecgemm<complex>(
		2, 2,
		core_layout_kind::vector_contiguous,
		core_layout_kind::vector_contiguous,
		core_layout_kind::row_major_contiguous
	);
	REQUIRE( vecgemm != nullptr );

	vecgemm(
		out.data(), left.data(), right.data(), out_core, left_core, right_core
	);

	const std::vector<complex> expected = { { 3, -4 }, { 1, 2 } };
	CHECK( out == expected );
}
