// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/numerical/fixed_width_float.hpp>

#include "float16_conversions.hpp"

namespace rex
{

float16_t::float16_t() noexcept
	: m_bits(0)
{
}

float16_t::float16_t(float value) noexcept
	: float16_t(float_to_float16(value))
{
}

float16_t::operator float() const noexcept
{
	return float16_to_float(*this);
}

std::uint16_t float16_t::get_bits() const noexcept
{
	return m_bits;
}

} // namespace rex
