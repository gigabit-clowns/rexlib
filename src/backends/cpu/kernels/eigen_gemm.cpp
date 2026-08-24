// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/kernels/eigen_gemm.hpp>

#include <cstddef>

namespace rexlib
{
namespace cpu
{

// Only the layout helpers live here. What resolve_gemm and its two
// siblings are defined by is heavy enough to want a translation unit per
// element type: see eigen_gemm_impl.hpp and the eigen_gemm_<type>.cpp
// files including it.

core_layout_kind classify_core_layout(
	const linalg_operand_core &core
) noexcept
{
	if (core.get_rank() == 1)
	{
		return core.get_stride(0) == 1
			? core_layout_kind::vector_contiguous
			: core_layout_kind::generic;
	}

	if (core.get_stride(1) == 1 &&
	    core.get_stride(0) == static_cast<std::ptrdiff_t>(core.get_extent(1)))
	{
		return core_layout_kind::row_major_contiguous;
	}
	if (core.get_stride(0) == 1 &&
	    core.get_stride(1) == static_cast<std::ptrdiff_t>(core.get_extent(0)))
	{
		return core_layout_kind::column_major_contiguous;
	}
	return core_layout_kind::generic;
}

bool is_row_major_compatible(core_layout_kind kind) noexcept
{
	return kind == core_layout_kind::row_major_contiguous ||
	       kind == core_layout_kind::vector_contiguous;
}

bool is_column_major_compatible(core_layout_kind kind) noexcept
{
	return kind == core_layout_kind::column_major_contiguous ||
	       kind == core_layout_kind::vector_contiguous;
}

linalg_operand_core pad_as_column(const linalg_operand_core &vector_core) noexcept
{
	return linalg_operand_core(
		2,
		{ vector_core.get_extent(0), 1 },
		{ vector_core.get_stride(0), 1 }
	);
}

linalg_operand_core pad_as_row(const linalg_operand_core &vector_core) noexcept
{
	return linalg_operand_core(
		2,
		{ 1, vector_core.get_extent(0) },
		{ 1, vector_core.get_stride(0) }
	);
}

} // namespace cpu
} // namespace rexlib
