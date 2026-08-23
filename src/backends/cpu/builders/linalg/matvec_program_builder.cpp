// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/linalg/matvec_operation.hpp>

#include <backends/cpu/builders/linalg_program_builder.hpp>
#include <backends/cpu/plans/linalg_core_layout_plan.hpp>
#include <backends/cpu/kernels/eigen_gemm.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>

#include <xmipp4/core/meta/type_list.hpp>

namespace xmipp4
{
namespace cpu
{

template <>
struct linalg_core_ranks<ops::matvec_operation>
{
	static constexpr std::size_t output = 1;
	static constexpr std::size_t left = 2;
	static constexpr std::size_t right = 1;
};

namespace
{

/**
 * @brief Multiply a (m, k) matrix by a k-vector, once per batch element.
 *
 * The vector operands are padded into (extent, 1) cores so the same gemm
 * implementations matmul uses serve here too: a vector has no major order
 * of its own, so this costs nothing in generality.
 */
template <typename T>
class matvec_core_kernel
{
public:
	matvec_core_kernel(
		const linalg_operand_core &out_core,
		const linalg_operand_core &left_core,
		const linalg_operand_core &right_core
	)
		: m_out_core(pad_as_column(out_core))
		, m_left_core(left_core)
		, m_right_core(pad_as_column(right_core))
		, m_gemm(resolve_gemv<T>(
			left_core.get_extent(0), left_core.get_extent(1),
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

struct matvec_kernel_factory
{
	template <typename Op, typename Out, typename In0, typename In1>
	auto operator()(
		const Op &/*operation*/,
		type_list<Out>,
		type_list<In0, In1>,
		const linalg_core_layout_plan &plan
	) const
	{
		return matvec_core_kernel<Out>(
			plan.get_output_core(),
			plan.get_left_core(),
			plan.get_right_core()
		);
	}
};

} // anonymous namespace

REX_REGISTER_LINALG_PROGRAM_BUILDER_EX(
	matvec,
	ops::matvec_operation,
	matvec_kernel_factory,
	rule_type_dispatcher<
		ops::matvec_operation::type_rule,
		eigen_scalar_support
	>
);

} // namespace cpu
} // namespace xmipp4
