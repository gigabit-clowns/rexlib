// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/creation/arange_operation.hpp>

#include <backends/cpu/builders/sequence_program_builder.hpp>
#include <backends/cpu/load_store.hpp>

#include <cstddef>

namespace rex
{
namespace cpu
{

namespace
{

template <typename T>
class arange_kernel
{
public:
	using compute_type = typename element_compute_type<T>::type;

	arange_kernel(compute_type start, compute_type step) noexcept
		: m_start(start)
		, m_step(step)
	{
	}

	void operator()(T *destination, std::size_t index) const noexcept
	{
		store(
			destination,
			m_start + static_cast<compute_type>(index) * m_step
		);
	}

private:
	compute_type m_start;
	compute_type m_step;
};

struct arange_kernel_factory
{
	template <typename T>
	arange_kernel<T> operator()(
		const ops::arange_operation &operation,
		type_list<T> /*output_types*/,
		type_list<> /*input_types*/
	) const
	{
		using compute_type = typename arange_kernel<T>::compute_type;
		return arange_kernel<T>(
			scalar_value_cast<compute_type>(operation.get_start()),
			scalar_value_cast<compute_type>(operation.get_step())
		);
	}
};

} // anonymous namespace

REXLIB_REGISTER_SEQUENCE_PROGRAM_BUILDER(
	arange,
	ops::arange_operation,
	arange_kernel_factory
);

} // namespace cpu
} // namespace rex
