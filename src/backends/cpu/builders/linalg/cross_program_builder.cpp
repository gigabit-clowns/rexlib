// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/linalg/cross_operation.hpp>

#include <backends/cpu/builders/dispatcher_support_query.hpp>
#include <backends/cpu/builders/linalg_program_builder.hpp>
#include <backends/cpu/builders/operand_data_types.hpp>
#include <backends/cpu/builders/type_dispatchers/rule_type_dispatcher.hpp>
#include <backends/cpu/hardware/functor_program.hpp>
#include <backends/cpu/kernels/eigen_gemm.hpp>
#include <backends/cpu/plans/linalg_core_layout_plan.hpp>

#include <rex/backends/cpu/program_builder.hpp>
#include <rex/core/dispatch/operand_signature.hpp>
#include <rex/core/dispatch/operation.hpp>
#include <rex/core/dispatch/operation_cast.hpp>
#include <rex/core/dispatch/program_builder_registry.hpp>
#include <rex/core/meta/type_list.hpp>

#include <core/dispatch/core_program_builder_registry.hpp>

#include <Eigen/Dense>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace rex
{
namespace cpu
{

namespace
{

/**
 * @brief Cross product of two three-component vectors, once per batch
 * element.
 *
 * The named axis always has extent three regardless of shape, so unlike
 * matmul/matvec/vecmat there is no size grid to dispatch: every instance
 * maps straight to Eigen's own cross product.
 */
template <typename T>
class cross_core_kernel
{
public:
	cross_core_kernel(
		const linalg_operand_core &out_core,
		const linalg_operand_core &left_core,
		const linalg_operand_core &right_core
	)
		: m_out_stride(out_core.get_stride(0))
		, m_left_stride(left_core.get_stride(0))
		, m_right_stride(right_core.get_stride(0))
	{
	}

	void operator()(T *out, const T *left, const T *right) const noexcept
	{
		using vector3 = Eigen::Matrix<T, 3, 1>;
		using stride_t = Eigen::InnerStride<Eigen::Dynamic>;

		Eigen::Map<const vector3, 0, stride_t> left_map(
			left, stride_t(m_left_stride)
		);
		Eigen::Map<const vector3, 0, stride_t> right_map(
			right, stride_t(m_right_stride)
		);
		Eigen::Map<vector3, 0, stride_t> out_map(
			out, stride_t(m_out_stride)
		);
		out_map.noalias() = left_map.cross(right_map);
	}

private:
	std::ptrdiff_t m_out_stride;
	std::ptrdiff_t m_left_stride;
	std::ptrdiff_t m_right_stride;
};

using cross_type_dispatcher =
	rule_type_dispatcher<ops::cross_operation::type_rule, eigen_scalar_support>;

/**
 * @brief CPU program builder for cross.
 *
 * Not built from linalg_program_builder: cross's core axis is a caller
 * chosen parameter rather than always the trailing one, so it uses
 * linalg_core_layout_plan::for_named_axis instead of for_trailing_core and
 * needs the operation's own axis at build() time.
 */
class cross_program_builder final
	: public program_builder
{
public:
	cross_program_builder() noexcept = default;
	~cross_program_builder() override = default;

	operation_id get_operation_id() const noexcept override
	{
		return operation_id::of<ops::cross_operation>();
	}

	backend_priority get_suitability(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		rex::command_queue &queue
	) const override
	{
		constexpr auto output_count = ops::cross_operation::output_operand_count;
		constexpr auto input_count = ops::cross_operation::input_operand_count;

		const auto base = program_builder::get_suitability(
			operation, output_signatures, input_signatures, queue
		);
		if (base == backend_priority::unsupported)
		{
			return base;
		}

		if (output_signatures.size() != output_count ||
		    input_signatures.size() != input_count)
		{
			return backend_priority::unsupported;
		}

		std::array<numerical_type, output_count> output_types;
		std::array<numerical_type, input_count> input_types;
		extract_data_types(output_types, output_signatures);
		extract_data_types(input_types, input_signatures);

		const auto supported =
			detail::dispatcher_support_query<cross_type_dispatcher>::is_supported(
				make_span(output_types.data(), output_count),
				make_span(input_types.data(), input_count)
			);
		if (!supported)
		{
			return backend_priority::unsupported;
		}

		return base;
	}

	std::shared_ptr<rex::program> build(
		const operation &operation,
		span<const operand_signature> output_signatures,
		span<const operand_signature> input_signatures,
		rex::command_queue& /*queue*/,
		program_cache* /*cache*/
	) const override
	{
		constexpr auto output_count = ops::cross_operation::output_operand_count;
		constexpr auto input_count = ops::cross_operation::input_operand_count;

		if (output_signatures.size() != output_count)
		{
			throw std::invalid_argument(
				"cross_program_builder::build: Unexpected output signature "
				"count."
			);
		}
		if (input_signatures.size() != input_count)
		{
			throw std::invalid_argument(
				"cross_program_builder::build: Unexpected input signature "
				"count."
			);
		}

		const auto &typed_operation =
			operation_cast<ops::cross_operation>(operation);
		const auto axis = typed_operation.get_shape_policy().get_axis();

		std::array<numerical_type, output_count> output_types;
		std::array<numerical_type, input_count> input_types;
		extract_data_types(output_types, output_signatures);
		extract_data_types(input_types, input_signatures);

		auto plan = linalg_core_layout_plan::for_named_axis(
			output_signatures, input_signatures, axis
		);

		return cross_type_dispatcher::dispatch(
			ops::cross_operation::get_static_descriptor(),
			[&plan] (auto output_element_types, auto input_element_types)
			{
				using out_type = typename type_list_element<
					0, 
					decltype(output_element_types)
				>::type;
				cross_core_kernel<out_type> kernel(
					plan.get_output_core(),
					plan.get_left_core(),
					plan.get_right_core()
				);
				using loop_functor_type = linalg_loop_functor<
					decltype(kernel),
					decltype(output_element_types),
					decltype(input_element_types)
				>;
				return make_functor_program(
					loop_functor_type(std::move(kernel), std::move(plan)),
					output_element_types,
					input_element_types
				);
			},
			output_types,
			input_types
		);
	}
};

} // anonymous namespace

static const program_builder_registration<cross_program_builder>
cross_program_builder_registration(get_core_program_builder_registry());

} // namespace cpu
} // namespace rex
