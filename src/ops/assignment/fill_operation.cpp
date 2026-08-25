// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/ops/assignment/fill_operation.hpp>

namespace rexlib
{
namespace ops
{

fill_operation::fill_operation(const scalar_value &fill_value) noexcept
	: m_fill_value(fill_value)
{
}

const scalar_value& fill_operation::get_fill_value() const noexcept
{
	return m_fill_value;
}

} // namespace ops
} // namespace rexlib
