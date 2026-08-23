// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/linalg/vecmat_operation.hpp>

#include <backends/cpu/builders/linalg_program_builder.hpp>
#include <backends/cpu/plans/linalg_core_layout_plan.hpp>
#include <backends/cpu/kernels/eigen_gemm.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <rex/core/meta/type_list.hpp>

namespace rex
{
namespace cpu
{

template <>
struct linalg_core_ranks<ops::vecmat_operation>
{
	static constexpr std::size_t output = 1;
	static constexpr std::size_t left = 1;
	static constexpr std::size_t right = 2;
};

namespace
{

/**
 * @brief Multiply a conjugated k-vector by a (k, n) matrix, once per batch
 * element.
 *
 * The mirror image of matvec_core_kernel: the vector operands are padded
 * into (1, extent) cores instead of (extent, 1) ones. Unlike matvec, the
 * vector is conjugated before the product (resolve_vecgemm's doing, not
 * this kernel's), matching NumPy's vecmat.
 */
template <typename T>
class vecmat_core_kernel
{
public:
	vecmat_core_kernel(
		const linalg_operand_core &out_core,
		const linalg_operand_core &left_core,
		const linalg_operand_core &right_core
	)
		: m_out_core(pad_as_row(out_core))
		, m_left_core(pad_as_row(left_core))
		, m_right_core(right_core)
		, m_gemm(resolve_vecgemm<T>(
			right_core.get_extent(0), right_core.get_extent(1),
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

struct vecmat_kernel_factory
{
	template <typename Op, typename Out, typename In0, typename In1>
	auto operator()(
		const Op &/*operation*/,
		type_list<Out>,
		type_list<In0, In1>,
		const linalg_core_layout_plan &plan
	) const
	{
		return vecmat_core_kernel<Out>(
			plan.get_output_core(),
			plan.get_left_core(),
			plan.get_right_core()
		);
	}
};

} // anonymous namespace

REX_REGISTER_LINALG_PROGRAM_BUILDER_EX(
	vecmat,
	ops::vecmat_operation,
	vecmat_kernel_factory,
	rule_type_dispatcher<
		ops::vecmat_operation::type_rule,
		eigen_scalar_support
	>
);

} // namespace cpu
} // namespace rex
