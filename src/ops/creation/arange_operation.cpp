// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/ops/creation/arange_operation.hpp>

namespace rex
{
namespace ops
{

arange_operation::arange_operation(
	const scalar_value &start,
	const scalar_value &step,
	std::size_t count
)
	: parametric_operation(count)
	, m_start(start)
	, m_step(step)
{
}

const scalar_value& arange_operation::get_start() const noexcept
{
	return m_start;
}

const scalar_value& arange_operation::get_step() const noexcept
{
	return m_step;
}

std::size_t arange_operation::get_count() const noexcept
{
	return get_shape_policy().get_count();
}

} // namespace ops
} // namespace rex
