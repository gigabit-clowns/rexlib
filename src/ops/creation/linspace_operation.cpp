// SPDX-License-Identifier: GPL-3.0-only

#include <rex/ops/creation/linspace_operation.hpp>

namespace rexlib
{
namespace ops
{

linspace_operation::linspace_operation(
	const scalar_value &start,
	const scalar_value &stop,
	std::size_t count,
	bool endpoint
)
	: parametric_operation(count)
	, m_start(start)
	, m_stop(stop)
	, m_endpoint(endpoint)
{
}

const scalar_value& linspace_operation::get_start() const noexcept
{
	return m_start;
}

const scalar_value& linspace_operation::get_stop() const noexcept
{
	return m_stop;
}

std::size_t linspace_operation::get_count() const noexcept
{
	return get_shape_policy().get_count();
}

bool linspace_operation::get_endpoint() const noexcept
{
	return m_endpoint;
}

} // namespace ops
} // namespace rexlib
