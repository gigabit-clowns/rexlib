// SPDX-License-Identifier: GPL-3.0-only

#include "sample_arange_program.hpp"

namespace sample
{

sample_arange_program::sample_arange_program(
	std::int32_t start,
	std::int32_t step,
	std::size_t count,
	std::ptrdiff_t offset,
	std::ptrdiff_t stride
) noexcept
	: m_start(start)
	, m_step(step)
	, m_count(count)
	, m_offset(offset)
	, m_stride(stride)
{
}

void sample_arange_program::execute(
	rexlib::span<const std::shared_ptr<rexlib::buffer>> outputs,
	rexlib::span<const std::shared_ptr<const rexlib::buffer>> /*inputs*/,
	rexlib::span<const std::shared_ptr<rexlib::buffer>> /*scratch*/
) const
{
	auto *values = static_cast<std::int32_t*>(outputs[0]->get_host_ptr());
	for (std::size_t i = 0; i < m_count; ++i)
	{
		const auto index = static_cast<std::ptrdiff_t>(i);
		values[m_offset + index * m_stride] = static_cast<std::int32_t>(
			m_start + static_cast<std::int64_t>(index) * m_step
		);
	}
}

} // namespace sample
