// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/optics/ctf_2d_operation.hpp>

#include <backends/cpu/em/parameters/ctf_optics/contrast_transfer_function.hpp>
#include <backends/cpu/loops/indices/multidimensional_index.hpp>
#include <backends/cpu/skeletons/elementwise/batched_map_plan.hpp>
#include <backends/cpu/skeletons/elementwise/elementwise_program_builder.hpp>

#include <rexlib/core/meta/type_list.hpp>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <cstddef>
#include <tuple>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief Makes the CTF of every image from its parameters.
 */
class ctf_2d_kernel
{
public:
	ctf_2d_kernel(
		const em::ctf_optics &optics,
		double pixel_size,
		std::size_t extent,
		std::ptrdiff_t parameter_stride
	) noexcept
		: m_optics(optics)
		, m_pixel_size(pixel_size)
		, m_extent(extent)
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
		const contrast_transfer_function<T> ctf(
			m_optics,
			m_pixel_size,
			m_extent,
			at(0),
			at(1),
			at(2),
			at(3)
		);
		const auto extent = static_cast<std::ptrdiff_t>(m_extent);
		const auto first_negative = (extent + 1) / 2;
		return [ctf, extent, first_negative](
			T *result,
			const multidimensional_index &index
		)
		{
			const auto row = static_cast<std::ptrdiff_t>(index[0]);
			const auto k_y = row < first_negative ? row : row - extent;
			*result = ctf(static_cast<T>(index[1]), static_cast<T>(k_y));
		};
	}

private:
	em::ctf_optics m_optics;
	double m_pixel_size;
	std::size_t m_extent;
	std::ptrdiff_t m_parameter_stride;
};

struct ctf_2d_kernel_factory
{
	template <typename... Outs, typename... Ins>
	ctf_2d_kernel operator()(
		const em::ctf_2d_operation &operation,
		type_list<Outs...>,
		type_list<Ins...>,
		const batched_map_plan &plan
	) const
	{
		return ctf_2d_kernel(
			operation.get_optics(),
			operation.get_pixel_size(),
			operation.get_parameters().get_extent(),
			plan.get_input_core(0).get_stride(0)
		);
	}
};

} // anonymous namespace

REXLIB_REGISTER_MULTIDIMENSIONAL_INDEXED_ELEMENTWISE_PROGRAM_BUILDER(
	ctf_2d,
	em::ctf_2d_operation,
	ctf_2d_kernel_factory
);

} // namespace cpu
} // namespace rexlib
