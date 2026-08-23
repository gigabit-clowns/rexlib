// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/hardware/program_scratch_requirement.hpp>

namespace rex
{

program_scratch_requirement::program_scratch_requirement(
	std::size_t size,
	std::size_t alignment,
	memory_resource_affinity affinity
) noexcept
	: m_size(size)
	, m_alignment(alignment)
	, m_affinity(affinity)
{
}

std::size_t program_scratch_requirement::get_size() const noexcept
{
	return m_size;
}

std::size_t program_scratch_requirement::get_alignment() const noexcept
{
	return m_alignment;
}

memory_resource_affinity program_scratch_requirement::get_affinity() const
	noexcept
{
	return m_affinity;
}

} // namespace rex
