// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/optics/ctf_1d_operation.hpp>

#include <backends/cpu/em/parameters/ctf_optics/contrast_transfer_function_1d.hpp>
#include <backends/cpu/em/parameters/ctf_optics/ctf_phase_coefficients.hpp>
#include <backends/cpu/loops/indices/multidimensional_index.hpp>
#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>
#include <backends/cpu/skeletons/elementwise/elementwise_program_builder.hpp>

#include <rexlib/core/meta/type_list.hpp>

#include <cstddef>
#include <tuple>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief Makes the CTF of every signal from its parameters.
 */
class ctf_1d_kernel
{
public:
	ctf_1d_kernel(
		const ctf_phase_coefficients &coefficients,
		std::ptrdiff_t parameter_stride
	) noexcept
		: m_coefficients(coefficients)
		, m_parameter_stride(parameter_stride)
	{
	}

	template <typename T>
	auto make_map(const std::tuple<const T*> &inputs) const
	{
		const auto *parameters = std::get<0>(inputs);
		const auto at = [this, parameters](std::ptrdiff_t i)
		{
			return static_cast<double>(parameters[i * m_parameter_stride]);
		};
		const contrast_transfer_function_1d<T> ctf(
			m_coefficients,
			at(0),
			at(1)
		);
		return [ctf](T *result, const multidimensional_index &index)
		{
			*result = ctf(static_cast<T>(index[0]));
		};
	}

private:
	ctf_phase_coefficients m_coefficients;
	std::ptrdiff_t m_parameter_stride;
};

struct ctf_1d_kernel_factory
{
	template <typename... Outs, typename... Ins>
	ctf_1d_kernel operator()(
		const em::ctf_1d_operation &operation,
		type_list<Outs...>,
		type_list<Ins...>,
		const batched_map_plan &plan
	) const
	{
		return ctf_1d_kernel(
			ctf_phase_coefficients(
				operation.get_optics(),
				operation.get_pixel_size(),
				operation.get_parameters().get_extent()
			),
			plan.get_input_core(0).get_stride(0)
		);
	}
};

} // anonymous namespace

REXLIB_REGISTER_MULTIDIMENSIONAL_INDEXED_ELEMENTWISE_PROGRAM_BUILDER(
	ctf_1d,
	em::ctf_1d_operation,
	ctf_1d_kernel_factory
);

} // namespace cpu
} // namespace rexlib
