// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/assignment/fill_operation.hpp>

#include <backends/cpu/builders/elementwise_program_builder.hpp>
#include <backends/cpu/load_store.hpp>

namespace rex
{
namespace cpu
{

namespace
{

template <typename T>
class fill_kernel
{
public:
	explicit fill_kernel(T value)
		: m_value(value)
	{
	}

	void operator()(T *result) const noexcept
	{
		store(result, m_value);
	}

private:
	T m_value;
};

struct fill_kernel_factory
{
	template <typename T>
	fill_kernel<T> operator()(
		const ops::fill_operation &operation,
		type_list<T> /*output_types*/,
		type_list<> /*input_types*/
	) const
	{
		return fill_kernel<T>(scalar_value_cast<T>(operation.get_fill_value()));
	}
};

} // anonymous namespace

REXLIB_REGISTER_ELEMENTWISE_PROGRAM_BUILDER(
	fill,
	ops::fill_operation,
	fill_kernel_factory
);

} // namespace cpu
} // namespace rex
