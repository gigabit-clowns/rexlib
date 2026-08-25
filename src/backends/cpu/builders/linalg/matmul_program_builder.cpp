// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/linalg/matmul_operation.hpp>

#include <backends/cpu/builders/linalg_program_builder.hpp>
#include <backends/cpu/plans/linalg_core_layout_plan.hpp>
#include <backends/cpu/kernels/eigen_gemm.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <rexlib/core/meta/type_list.hpp>

namespace rexlib
{
namespace cpu
{

template <>
struct linalg_core_ranks<ops::matmul_operation>
{
	static constexpr std::size_t output = 2;
	static constexpr std::size_t left = 2;
	static constexpr std::size_t right = 2;
};

namespace
{

/**
 * @brief Multiply a (m, k) matrix by a (k, n) matrix, once per batch
 * element.
 *
 * Resolves, once at construction, which Eigen mapping to use: a fully
 * unrolled fixed-size GEMM when every operand is contiguous of a shared
 * major order and every extent is at most four, or a fully dynamic one
 * otherwise. See eigen_gemm.hpp.
 */
template <typename T>
class matmul_core_kernel
{
public:
	matmul_core_kernel(
		const linalg_operand_core &out_core,
		const linalg_operand_core &left_core,
		const linalg_operand_core &right_core
	)
		: m_out_core(out_core)
		, m_left_core(left_core)
		, m_right_core(right_core)
		, m_gemm(resolve_gemm<T>(
			left_core.get_extent(0), left_core.get_extent(1),
			right_core.get_extent(1),
			classify_core_layout(out_core),
			classify_core_layout(left_core),
			classify_core_layout(right_core)
		))
	{
	}

	void operator()(T *out, const T *left, const T *right) const noexcept
	{
		m_gemm(out, left, right, m_out_core, m_left_core, m_right_core);
	}

private:
	linalg_operand_core m_out_core;
	linalg_operand_core m_left_core;
	linalg_operand_core m_right_core;
	gemm_fn<T> m_gemm;
};

struct matmul_kernel_factory
{
	template <typename Op, typename Out, typename In0, typename In1>
	auto operator()(
		const Op &/*operation*/,
		type_list<Out>,
		type_list<In0, In1>,
		const linalg_core_layout_plan &plan
	) const
	{
		return matmul_core_kernel<Out>(
			plan.get_output_core(),
			plan.get_left_core(),
			plan.get_right_core()
		);
	}
};

} // anonymous namespace

REXLIB_REGISTER_LINALG_PROGRAM_BUILDER_EX(
	matmul,
	ops::matmul_operation,
	matmul_kernel_factory,
	rule_type_dispatcher<
		ops::matmul_operation::type_rule,
		eigen_scalar_support
	>
);

} // namespace cpu
} // namespace rexlib
